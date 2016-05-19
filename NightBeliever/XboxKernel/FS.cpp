#include "Kernel.hpp"

NTSTATUS NTAPI kernel_NtOpenFile(
	HANDLE *FileHandle, 
	ACCESS_MASK DesiredAccess, 
	OBJECT_ATTRIBUTES *ObjectAttributes, 
	IO_STATUS_BLOCK *IoStatusBlock, 
	uint32_t ShareAccess, 
	uint32_t OpenOptions
) {
	log("NtOpenFile('%s', 0x%08x, 0x%08x)", ObjectAttributes->ObjectName->Buffer, DesiredAccess, OpenOptions);
	auto flags = FSFlags::READ;
	if(FLAG(DesiredAccess, FILE_WRITE_DATA))
		flags |= FSFlags::WRITE;
	if(FLAG(OpenOptions, FILE_DIRECTORY_FILE))
		flags |= FSFlags::DIRECTORY;
	
	*FileHandle = io_open(ObjectAttributes->RootDirectory, (char *) ObjectAttributes->ObjectName->Buffer, flags);
	if(*FileHandle == 0)
		return STATUS_OBJECT_NAME_NOT_FOUND;
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI kernel_NtCreateFile(
	OUT PHANDLE             FileHandle, 
	IN  ACCESS_MASK         DesiredAccess,
	IN  POBJECT_ATTRIBUTES  ObjectAttributes,
	OUT PIO_STATUS_BLOCK    IoStatusBlock,
	IN  PLARGE_INTEGER      AllocationSize, 
	IN  ULONG               FileAttributes, 
	IN  ULONG               ShareAccess, 
	IN  ULONG               CreateDisposition, 
	IN  ULONG               CreateOptions 
) {
	log("NtCreateFile('%s', 0x%08x, 0x%08x, 0x%08x)", 
		ObjectAttributes->ObjectName->Buffer, 
		DesiredAccess, 
		CreateDisposition, 
		CreateOptions
	);
	auto flags = FSFlags::READ;
	if(CreateDisposition == FILE_CREATE || CreateDisposition == FILE_OPEN_IF)
		flags |= FSFlags::CREATE;
	else if(CreateDisposition == FILE_OVERWRITE || CreateDisposition == FILE_OVERWRITE_IF)
		flags |= FSFlags::OVERWRITE;
	if(FLAG(DesiredAccess, FILE_WRITE_DATA))
		flags |= FSFlags::WRITE;
	if(FLAG(CreateOptions, FILE_DIRECTORY_FILE))
		flags |= FSFlags::DIRECTORY;
	*FileHandle = io_open(ObjectAttributes->RootDirectory, (char *) ObjectAttributes->ObjectName->Buffer, flags);
	if(*FileHandle == 0) {
		return STATUS_OBJECT_NAME_NOT_FOUND;
	}
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI kernel_NtReadFile(
	HANDLE          FileHandle,
	HANDLE          Event,
	PVOID           ApcRoutine,
	PVOID           ApcContext,
	PVOID           IoStatusBlock,
	PVOID           Buffer,
	ULONG           Length,
	PLARGE_INTEGER  ByteOffset
) {
	auto offset = (ByteOffset == NULL || ByteOffset->u.HighPart == -1) ? -1 : ByteOffset->QuadPart;
	if(io_read(FileHandle, Buffer, Length, offset))
		return STATUS_SUCCESS;
	bailout("Unknown failure");
	return -1;
}

NTSTATUS NTAPI kernel_NtWriteFile(
	HANDLE          FileHandle,
	HANDLE          Event,
	PVOID           ApcRoutine,
	PVOID           ApcContext,
	PVOID           IoStatusBlock,
	PVOID           Buffer,
	ULONG           Length,
	PLARGE_INTEGER  ByteOffset
) {
	auto offset = (ByteOffset == NULL || ByteOffset->u.HighPart == -1) ? -1 : ByteOffset->QuadPart;
	if(io_write(FileHandle, Buffer, Length, offset))
		return STATUS_SUCCESS;
	bailout("Unknown failure");
	return -1;
}

NTSTATUS NTAPI kernel_NtDeviceIoControlFile(
	HANDLE          FileHandle,
	HANDLE          Event,
	PVOID           ApcRoutine,
	PVOID           ApcContext,
	PVOID           IoStatusBlock,
	ULONG           IoControlCode,
	PVOID           InputBuffer,
	ULONG           InputLength,
	PVOID           OutputBuffer,
	ULONG           OutputLength
) {
	if(io_ioctl(FileHandle, IoControlCode, InputBuffer, InputLength, OutputBuffer, OutputLength))
		return STATUS_SUCCESS;
	bailout("Unknown failure");
	return -1;
}

NTSTATUS NTAPI kernel_NtClose(HANDLE handle) {
	close(handle);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI kernel_NtQueryVolumeInformationFile(
	IN  HANDLE                      FileHandle,
	OUT PIO_STATUS_BLOCK            IoStatusBlock,
	OUT PVOID                       FileInformation,
	IN  ULONG                       Length,
	IN  FS_INFORMATION_CLASS        FileInformationClass
) {
	if(IoStatusBlock)
		debug("Ignoring status block");
	
	switch(FileInformationClass) {
		case FileFsSizeInformation: {
			log("FsSizeInformation");
			auto info = (FILE_FS_SIZE_INFORMATION *) FileInformation;
			// Test XBE *requires* bytes per allocation unit == 16kb!
			info->BytesPerSector = 4096; // 4KB sectors
			info->SectorsPerAllocationUnit = 4; // 16KB allocation units
			info->TotalAllocationUnits.QuadPart = 524288; // 8GB drive
			info->AvailableAllocationUnits.QuadPart = 458752; // 7GB free
			break;
		}
		default:
			bailout("Unknown FS_INFORMATION_CLASS %i", FileInformationClass);
	}
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI kernel_IoCreateSymbolicLink(
	IN PSTRING SymbolicLinkName,
	IN PSTRING DeviceName
) {
	log("IoCreateSymbolicLink('%s', '%s')", SymbolicLinkName->Buffer, DeviceName->Buffer);
	return STATUS_SUCCESS;
}

uint32_t kernel_HalDiskCachePartitionCount = 1;
