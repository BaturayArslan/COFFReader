#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// using uint16_t and uint32_t because short, int and long sizes could be different depending on a platform and a compiler. Since, on fixed size headers its critical to be exact with sizes I'm using "fixed" size data types here for all different platforms.
// 
struct FILE_HEADER
{
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
};

struct SECTION_HEADER
{
    char       Name[8];
    uint32_t   VirtualSize;
    uint32_t   VirtualAddress;
    uint32_t   SizeOfRawData;
    uint32_t   PointerToRawData;
    uint32_t   PointerToRelocations;
    uint32_t   PointerToLinenumbers;
    uint16_t   NumberOfRelocations;
    uint16_t   NumberOfLinenumbers;
    uint32_t   Characteristics;
};

void ShowError(const char *messageFormat, const char *data )
{
    fprintf(stderr, messageFormat, data);
}

void VerifyArg(int argc, char **argv)
{
    if(argc < 2)
    {
        ShowError("Usage: %s <filePath> \n", argv[0]);
        exit(1);                                    // User didn't supply the object file path
    }
}

const char* GetObjFilePath(char **argv)
{
    return argv[1];
}

FILE* OpenObjectFile(const char *path)
{
    FILE* file = fopen(path, "rb");
    if(!file)
    {
        ShowError("File couldn't found: %s", path);
        exit(1);
    }
    return file;
}

void ReadFileHeader(FILE* file, FILE_HEADER *fileHeader)
{
    size_t count = fread(fileHeader, sizeof(FILE_HEADER), 1, file);
    if(count != 1)
     {
         ShowError("Something went wrong during reading file header.", "");
         exit(1);
     }
}

 void PrintSeparator()
 {
     printf("+------------------------+--------------------+\n");
 }

 void PrintRow(const char *field, const char *value)
 {
     printf("| %-22s | %-18s |\n", field, value);
 }

 void PrintFileHeader(const FILE_HEADER *h)
 {
     char buf[32];

     PrintSeparator();
     printf("| %-43s |\n", "COFF FILE HEADER");
     PrintSeparator();

     snprintf(buf, sizeof(buf), "0x%04X", h->Machine);
     PrintRow("Machine", buf);

     snprintf(buf, sizeof(buf), "%u", h->NumberOfSections);
     PrintRow("NumberOfSections", buf);

     snprintf(buf, sizeof(buf), "0x%08X", h->TimeDateStamp);
     PrintRow("TimeDateStamp", buf);

     snprintf(buf, sizeof(buf), "0x%08X", h->PointerToSymbolTable);
     PrintRow("PointerToSymbolTable", buf);

     snprintf(buf, sizeof(buf), "%u", h->NumberOfSymbols);
     PrintRow("NumberOfSymbols", buf);

     snprintf(buf, sizeof(buf), "%u", h->SizeOfOptionalHeader);
     PrintRow("SizeOfOptionalHeader", buf);

     snprintf(buf, sizeof(buf), "0x%04X", h->Characteristics);
     PrintRow("Characteristics", buf);

     PrintSeparator();
 }

void ReadSectionHeaders(FILE* file,SECTION_HEADER *headers, unsigned short count)
{
    for(int i = 0; i < count; i++)
    {
        size_t read = fread((headers+i), sizeof(SECTION_HEADER), 1, file);
        if(read != 1)
        {
            ShowError("Something went wrong during reading file section.", "");
            exit(1);
        }    
    }
}

void PrintSectionHeader(const SECTION_HEADER *s, int index)
{
    char buf[32];
    char title[32];

    snprintf(title, sizeof(title), "SECTION HEADER #%d", index);

    PrintSeparator();
    printf("| %-43s |\n", title);
    PrintSeparator();

    snprintf(buf, sizeof(buf), "%.8s", s->Name);          // Name isn't null-terminated when it uses all 8 bytes
    PrintRow("Name", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->VirtualSize);
    PrintRow("VirtualSize", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->VirtualAddress);
    PrintRow("VirtualAddress", buf);

    snprintf(buf, sizeof(buf), "%u", s->SizeOfRawData);
    PrintRow("SizeOfRawData", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->PointerToRawData);
    PrintRow("PointerToRawData", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->PointerToRelocations);
    PrintRow("PointerToRelocations", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->PointerToLinenumbers);
    PrintRow("PointerToLinenumbers", buf);

    snprintf(buf, sizeof(buf), "%u", s->NumberOfRelocations);
    PrintRow("NumberOfRelocations", buf);

    snprintf(buf, sizeof(buf), "%u", s->NumberOfLinenumbers);
    PrintRow("NumberOfLinenumbers", buf);

    snprintf(buf, sizeof(buf), "0x%08X", s->Characteristics);
    PrintRow("Characteristics", buf);

    PrintSeparator();
}

void PrintSectionHeaders(const SECTION_HEADER *headers, unsigned short count)
{
    for (int i = 0; i < count; i++)
    {
        PrintSectionHeader(&headers[i], i + 1);
    }
}

int main(int argc, char **argv)
{
    VerifyArg(argc, argv);
    const char *objFilePath = GetObjFilePath(argv);

    FILE *file = OpenObjectFile(objFilePath);

    // Read file header
    FILE_HEADER fileHeader;
    ReadFileHeader(file, &fileHeader);
     PrintFileHeader(&fileHeader);

    // advance the file cursor
    fseek(file, fileHeader.SizeOfOptionalHeader, SEEK_CUR);

    // read sections headers
    unsigned short numberOfSections = fileHeader.NumberOfSections;
    SECTION_HEADER *sectionHeaders = (SECTION_HEADER *) malloc(numberOfSections * sizeof(SECTION_HEADER));
    ReadSectionHeaders(file, sectionHeaders, numberOfSections);
    PrintSectionHeaders(sectionHeaders, numberOfSections);


    free(sectionHeaders);
    sectionHeaders = NULL;
    fclose(file);
    return 0;
}
 
