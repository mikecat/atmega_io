#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidpi.h>
#include <stdlib.h>
#include "usbio_windows.h"
#include "avr_io.h"

#define IO_SIZE 65
#define PORT_NUM 12

typedef struct {
	HANDLE hDevice;
	int sin_port, sout_port, reset_port;
} hid_t;

static int openHID(HANDLE *hHid, int vendor_id, const int product_ids[], int product_id_num) {
	GUID hid_guid;
	HDEVINFO hDeviceInfo;
	int index;
	HidD_GetHidGuid(&hid_guid);
	if (product_ids == NULL || product_id_num <= 0) return 0;

	/* デバイスを列挙して探す */
	hDeviceInfo = SetupDiGetClassDevs(
		&hid_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	for (index = 0; ; index++) {
		SP_DEVICE_INTERFACE_DATA did;
		SP_DEVICE_INTERFACE_DETAIL_DATA *detail;
		DWORD needed_size;
		did.cbSize = sizeof(did);
		/* 次のデバイスの情報を得る準備をする */
		if (SetupDiEnumDeviceInterfaces(hDeviceInfo, NULL, &hid_guid, index, &did)) {
			SP_DEVINFO_DATA devinfo;
			HANDLE hDevice;
			/* このデバイスの情報を得る */
			devinfo.cbSize = sizeof(devinfo);
			if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &did, NULL, 0, &needed_size, &devinfo)
				&& GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;
			detail = HeapAlloc(GetProcessHeap(), 0, needed_size);
			if (detail == NULL) break;
			detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo, &did, detail, needed_size, &needed_size, &devinfo)) {
				HeapFree(GetProcessHeap(), 0, detail);
				break;
			}
			/* このデバイスを開く */
			hDevice = CreateFile(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
			HeapFree(GetProcessHeap(), 0, detail);
			if (hDevice != INVALID_HANDLE_VALUE) {
				HIDD_ATTRIBUTES attr;
				/* IDがマッチするかを調べる */
				if (HidD_GetAttributes(hDevice, &attr) && attr.VendorID == vendor_id) {
					int i, matched = 0;
					for (i = 0; i < product_id_num; i++) {
						if (attr.ProductID == product_ids[i]) {
							matched = 1;
							break;
						}
					}
					if (matched) {
						/* 最終的な情報を記録する */
						PHIDP_PREPARSED_DATA ppd;
						HIDP_CAPS caps;
						if (HidD_GetPreparsedData(hDevice, &ppd) &&
						HidP_GetCaps(ppd, &caps) == HIDP_STATUS_SUCCESS &&
						caps.OutputReportByteLength == IO_SIZE &&
						caps.InputReportByteLength == IO_SIZE) {
							/* 結果を返す */
							*hHid = hDevice;
							SetupDiDestroyDeviceInfoList(hDeviceInfo);
							return 1;
						}
					}
				}
				CloseHandle(hDevice);
			}
		} else {
			break;
		}
	}
	SetupDiDestroyDeviceInfoList(hDeviceInfo);
	return 0;
}

static int writeAndRead(HANDLE hHid,const unsigned char* writeData,unsigned char* readData) {
	DWORD size;
	if(!WriteFile(hHid,writeData,IO_SIZE,&size,NULL) || size!=IO_SIZE)return 0;
	if(!ReadFile(hHid,readData,IO_SIZE,&size,NULL) || size!=IO_SIZE)return 0;
	return 1;
}

static int inputAndOutput(HANDLE hUsbIO,int writeData,int *readData) {
	unsigned char write_buffer[IO_SIZE]={};
	unsigned char read_buffer[IO_SIZE]={};
	write_buffer[1]=0x20;
	write_buffer[2]=0x1;
	write_buffer[3]=writeData&0xff;
	write_buffer[4]=0x2;
	write_buffer[5]=(writeData>>8)&0x0f;
	if(!writeAndRead(hUsbIO,write_buffer,read_buffer))return 0;
	if(readData!=NULL)*readData=read_buffer[2]|(read_buffer[3]<<8);
	return 1;
}

int usbio_init(avrio_t *avrio, int sin_port, int sout_port, int reset_port) {
	static const int vendor_id = 0x1352;
	static const int product_id[2] = {0x120, 0x121};
	static const int product_id_num = 2;
	HANDLE hUsbIO;
	hid_t *hid;
	if (avrio == NULL) return 0;
	if (sin_port < 0 || PORT_NUM <= sin_port || sout_port < 0 || PORT_NUM <= sout_port ||
	reset_port < 0 || PORT_NUM <= reset_port || sin_port == sout_port ||
	sout_port == reset_port || reset_port == sin_port) {
		/* 無効なポートまたはポートが被っている */
		return 0;
	}
	/* USB-IO2.0を開く */
	if(!openHID(&hUsbIO, vendor_id, product_id, product_id_num)) {
		return 0;
	}
	/* 情報を格納する */
	hid = malloc(sizeof(hid_t));
	if (hid == NULL) return 0;
	hid->hDevice = hUsbIO;
	hid->sin_port = sin_port;
	hid->sout_port = sout_port;
	hid->reset_port = reset_port;
	avrio->hardware_data = (void*)hid;
	avrio->io_8bits = usbio_io_8bits;
	return 1;
}

int usbio_stop(avrio_t *avrio) {
	hid_t *hid;
	HANDLE hDevice;
	if (avrio == NULL) return 0;
	hid = (hid_t*)avrio->hardware_data;
	if (hid == NULL) return 0;
	hDevice = hid->hDevice;
	free(hid);
	if (!CloseHandle(hid->hDevice)) return 0;
	return 1;
}

int usbio_io_8bits(void *hardware_data, int out) {
	return -1;
}
