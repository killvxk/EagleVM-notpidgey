#include "pe/pe_generator.h"

PIMAGE_DOS_HEADER pe_generator::build_dos_header()
{
    constexpr auto dos_header_size = sizeof IMAGE_DOS_HEADER;

    ZeroMemory(&dos_header, dos_header_size);
    dos_header.e_magic = 'MZ';
    dos_header.e_lfanew = 0x40;

    return &dos_header;
}

PIMAGE_FILE_HEADER pe_generator::build_coff_header()
{
    constexpr auto coff_header_size = sizeof IMAGE_FILE_HEADER;

    ZeroMemory(&coff_header, coff_header_size);
    coff_header.Machine = IMAGE_FILE_MACHINE_AMD64;
    coff_header.NumberOfSections = 3; // TODO
    coff_header.TimeDateStamp = 0; // 1970-01-01 00:00:00
    coff_header.SizeOfOptionalHeader = 0; // TODO
    coff_header.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE;

    return &coff_header;
}

PIMAGE_OPTIONAL_HEADER pe_generator::build_optional_header(uint64_t stack_reserve, uint64_t stack_commit, uint64_t heap_reserve, uint64_t heap_commit)
{
    constexpr auto optional_header_size = sizeof IMAGE_OPTIONAL_HEADER;

    ZeroMemory(&optional_header, optional_header_size);
    optional_header.Magic = IMAGE_NT_OPTIONAL_HDR_MAGIC;
    optional_header.MajorLinkerVersion = 0xE;
    optional_header.MinorLinkerVersion = 0x1C;
    optional_header.SizeOfCode = 0; // TODO
    optional_header.SizeOfInitializedData = 0; // TODO
    optional_header.AddressOfEntryPoint = 0; // TODO
    optional_header.BaseOfCode = 1000;

    optional_header.ImageBase = default_image_base;
    optional_header.SectionAlignment = default_section_alignment;
    optional_header.FileAlignment = default_file_alignment;
    optional_header.MajorOperatingSystemVersion = 5; 
    optional_header.MinorOperatingSystemVersion = 3;
    optional_header.MajorImageVersion = 0;
    optional_header.MinorImageVersion = 0;
    optional_header.MajorSubsystemVersion = 5;
    optional_header.MinorSubsystemVersion = 3;
    optional_header.Win32VersionValue = 0;
    optional_header.SizeOfImage = 0; // TODO
    optional_header.CheckSum = 0; // TODO
    optional_header.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    optional_header.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY |
        IMAGE_DLLCHARACTERISTICS_NX_COMPAT | IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE;
    optional_header.SizeOfStackReserve = stack_reserve;
    optional_header.SizeOfStackCommit = stack_commit;
    optional_header.SizeOfHeapReserve = heap_reserve;
    optional_header.SizeOfHeapCommit = heap_commit;
    optional_header.LoaderFlags = 0;
    optional_header.NumberOfRvaAndSizes = 10;

    return &optional_header;
}

std::vector<char>& pe_generator::build_section_table()
{
    std::vector<char> test;

    return test;
}
