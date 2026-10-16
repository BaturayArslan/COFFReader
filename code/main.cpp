#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

#pragma pack(push, 1)
struct RELOCATION
{
    uint32_t VirtualAddress;
    uint32_t SymbolTableIndex;
    uint16_t Type;
};
# pragma pack(pop)

#pragma pack(push, 1)
struct SYMBOL
{
    union {
        char    ShortName[8];
        struct {
            uint32_t   Short;     // if 0, use LongName
            uint32_t   Long;      // offset into string table
        } Name;
    } N;
    uint32_t    Value;
    int16_t     SectionNumber;
    uint16_t    Type;
    uint8_t     StorageClass;
    uint8_t     NumberOfAuxSymbols;
};
#pragma pack(pop)

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

char* ReadSectionText(FILE* file, SECTION_HEADER *directiveHeader)
{
    char *buffer = (char*) malloc(directiveHeader->SizeOfRawData + 1);
    fseek(file, directiveHeader->PointerToRawData, SEEK_SET);
    size_t count = fread(buffer, directiveHeader->SizeOfRawData, 1, file);
    if(count != 1){
        ShowError("Something went wrong reading directive header.", "");
    }

    buffer[directiveHeader->SizeOfRawData] = '\0';
    return buffer;
}

int compareStrings(char* first, char* second, int upto = 0)
{
    int index = 0;
    while(first[index] != '\0' && second[index] != '\0')
    {
        if(first[index] != second[index])
        {
            // They are not same
            return 0;
        }

        index += 1;
        if(upto != 0 && upto <= index)
        {
            return 1;
        }
    }

    if(first[index] != second[index])
    {
        return 0;
    }

    return 1;
}

SECTION_HEADER *findSectionHeader(char *name, SECTION_HEADER* headerList, int listSize)
{
    for(int i = 0; i < listSize; i++)
    {
        if(compareStrings(name, headerList[i].Name, 8))
        {
            return &headerList[i];
        }
    }

    return NULL;
}

RELOCATION *ReadRelocationTable(FILE* file, char* sectionName, SECTION_HEADER* headerList, int listSize)
{

    SECTION_HEADER *header = findSectionHeader(sectionName, headerList, listSize);      
    if(!header)
    {
        ShowError("Something went wrong finding section header in ReadRelocationTable function.", "");
    }
    
    int tableEntryCount = header->NumberOfRelocations;
    if(tableEntryCount == 0)
    {
        return NULL;
    }
    RELOCATION* relocationTable = (RELOCATION* ) malloc(tableEntryCount * sizeof(RELOCATION));
    if(!relocationTable)
    {
        ShowError("Out of memory!", "");
    }
        
    fseek(file, header->PointerToRelocations, SEEK_SET);
    size_t count = fread(relocationTable, sizeof(RELOCATION), tableEntryCount, file);
    if(count != tableEntryCount)
    {
        ShowError("Something went wrong reading relocation table.", "");
    }

    return relocationTable;
}

void PrintRelocations(const SECTION_HEADER *section, const RELOCATION *table)
{
    char title[64];
    snprintf(title, sizeof(title), "RELOCATIONS: %.8s (%u)", section->Name, section->NumberOfRelocations);

    printf("+-------+------------+--------------+--------+\n");
    printf("| %-42s |\n", title);
    printf("+-------+------------+--------------+--------+\n");

    if (!table)
    {
        printf("| %-42s |\n", "(none)");
        printf("+-------+------------+--------------+--------+\n");
        return;
    }

    printf("| %-5s | %-10s | %-12s | %-6s |\n", "#", "Offset", "SymbolIndex", "Type");
    printf("+-------+------------+--------------+--------+\n");

    for (unsigned i = 0; i < section->NumberOfRelocations; i++)
    {
        printf("| %-5u | 0x%08X | %-12u | 0x%04X |\n",
               i, table[i].VirtualAddress, table[i].SymbolTableIndex, table[i].Type);
    }

    printf("+-------+------------+--------------+--------+\n");
}

void ReplaceCharacter(char *text, char replace, char replaceWith)
{
    unsigned int index = 0;
    while(text[index] != '\0')
    {
        if(text[index] == replace)
        {
            text[index] = replaceWith;
        }
        index += 1;
    }
}

char **divideCommand(char* text, char delimiter)
{
    char **result = (char **) malloc(4 * sizeof(char*));
    for(int i = 0; i < 4; i ++)
    {
        result[i] = NULL;
    }
    unsigned int resultIndex = 0;

    unsigned int startIndex = 0;
    for(int i = 0; text[i] != '\0'; i++)
    {
        if(text[i] == delimiter || text[i+1] == '\0')
        {
            unsigned int substringSize = i - startIndex;
            // special length calculation for text[i+1] case.
            if(text[i+1] == '\0')
            {
                substringSize = i + 1 - startIndex;
            }
            
            if(substringSize == 0)
            {
                startIndex = i + 1;
                continue;   
            }
            
            // +1 because of extra \0 at the end.
            char* substring = (char *) malloc(substringSize + 1);
            for(int j = 0; j < substringSize; j++)
            {
                substring[j] = text[startIndex + j];
            }
            substring[substringSize] = '\0';
            result[resultIndex] = substring;
            resultIndex += 1;
            startIndex = i + 1;
        }
    }

    return result;
}

SYMBOL *ReadSymbols(FILE* file, unsigned int pointerToSymbols, unsigned int numberOfSymbols)
{
    // printf("pointerToSymbols: %u \n", pointerToSymbols);
    // printf("numberOfSymbols: %u \n", numberOfSymbols);
    fseek(file, pointerToSymbols, SEEK_SET);
    SYMBOL *symbolList = (SYMBOL *) malloc(numberOfSymbols * sizeof(SYMBOL));
    size_t count = fread(symbolList, sizeof(SYMBOL), numberOfSymbols, file);
    if(count != numberOfSymbols)
    {
        fprintf(stderr ,"Something went wrong reading symbols. Read %zu count \n", count);
        exit(1);
    }
   
    return symbolList;
}

char* ReadStringTable(FILE* file, unsigned int stringTableOffset)
{
    fseek(file, stringTableOffset, SEEK_SET);
    
    uint32_t size = 0;
    size_t count = fread(&size, sizeof(size), 1, file);
    if(count != 1 || size < 4)
    {
        // Doesn't have string table
        return NULL;
    }

    char* table = (char *) malloc(size + 1);
    if(!table)
    {
        ShowError("out of memory for string table", "");
    }
    
    fseek(file, stringTableOffset, SEEK_SET);
    count = fread(table, 1, size, file);
    if(count != size)
    
    {
        ShowError("Something went wrong reading string table.", "");
        exit(1);
    }
    
    table[size] = '\0';
    return table;
}

char* GetSymbolName(SYMBOL symbol, char* stringTable)
{
    if(!stringTable)
    {
        ShowError("Reference to string table is NULL!!", "");
    }
    
    unsigned int length = 8;
    char* source = symbol.N.ShortName;
    if(!symbol.N.Name.Short)
    {
        unsigned int startIndex = symbol.N.Name.Long;
        unsigned int endIndex = startIndex;
        while(stringTable[endIndex] != '\0')
        {
            endIndex += 1;
        }
        length  = endIndex - startIndex;
        source = stringTable + symbol.N.Name.Long;
    }

    char *name = (char*) malloc(length + 1);
    memcpy(name, source, length);
    name[length] = '\0';
    return name;
        
}

void PrintSymbolSeparator()
{
    static const int widths[] = { 5, 10, 6, 6, 5, 3, 40 };      // # Value Sect Type Class Aux Name
    for (int i = 0; i < 7; i++)
    {
        putchar('+');
        for (int j = 0; j < widths[i] + 2; j++) putchar('-');
    }
    printf("+\n");
}

void PrintSymbols(const SYMBOL *symbols, uint32_t count, char *stringTable)
{
    char title[64];
    snprintf(title, sizeof(title), "SYMBOL TABLE (%u records)", count);

    PrintSymbolSeparator();
    printf("| %-93s |\n", title);
    PrintSymbolSeparator();
    printf("| %-5s | %-10s | %-6s | %-6s | %-5s | %-3s | %-40s |\n",
           "#", "Value", "Sect", "Type", "Class", "Aux", "Name");
    PrintSymbolSeparator();

    // an aux record has the same size as a symbol but is not one, so skip over them
    for (uint32_t i = 0; i < count; i += 1 + symbols[i].NumberOfAuxSymbols)
    {
        char *name = GetSymbolName(symbols[i], stringTable);
        printf("| %-5u | 0x%08X | %-6d | 0x%04X | %-5d | %-3d | %-40.40s |\n",
               i,
               symbols[i].Value,
               symbols[i].SectionNumber,
               symbols[i].Type,
               symbols[i].StorageClass,
               symbols[i].NumberOfAuxSymbols,
               name);
        free(name);
    }

    PrintSymbolSeparator();
}

char* ReadSectionBinary(FILE* file, SECTION_HEADER* header)
{
    fseek(file, header->PointerToRawData, SEEK_SET);
    char *content = (char*) malloc(header->SizeOfRawData + 1);
    size_t count = fread(content, 1, header->SizeOfRawData, file);
    if(count != header->SizeOfRawData)
    {
        ShowError("Something went wrong reading section data.", "");
        exit(1);
    }

    content[header->SizeOfRawData] = '\n';
    return content;
}


unsigned int GetStringLength(char *string)
{
    unsigned int length = 0;
    
    while(string[length] != '\0')
    {
        length += 1;
    }

    return length;
}

char *MergeStrings(char *first, char* second)
{
    unsigned int firstLength = GetStringLength(first);
    unsigned int secondLength = GetStringLength(second);


    char* mergeString = (char *) malloc(firstLength + secondLength + 1);
    memcpy(mergeString, first, firstLength);
    memcpy((mergeString + firstLength), second, secondLength);

    mergeString[firstLength + secondLength] = '\0';
    return mergeString;
}

void PrintTitle(char* title, unsigned int rowLength)
{
    putchar('+');
    for(int i = 0; i < rowLength - 2; i++)
    {
        putchar('-');
    }
    putchar('+');
    putchar('\n');

    int titleLength = GetStringLength(title);
    int leftPadding = ((rowLength - 2) - titleLength) / 2;
    if (leftPadding < 0) leftPadding = 0;
    unsigned int rightPadding = (rowLength - 2) - leftPadding - titleLength;
    if(rightPadding < 0) rightPadding = 0;
    
    printf("+%*s%s%*s+\n", leftPadding, "", title, rightPadding, "");

    putchar('+');
    for(int i = 0; i < rowLength - 2; i++)
    {
        putchar('-');
    }
    putchar('+');
    putchar('\n');
    
}

void PrintSectionBinary(char* content, SECTION_HEADER *header)
{
    unsigned int TITLE_ROW_LENGTH = 80;
    unsigned int ADDRESS_FIELD_LENGTH = 10;
    unsigned int BYTE_IN_ROW = 16;
    unsigned int BYTE_FIELD_LENGTH = 3;       // Don't give lower than 2.
    
    
    // be carefull header->Name is not always '\0' terminated.
    char* title = MergeStrings(header->Name, " Section");
    PrintTitle(title, TITLE_ROW_LENGTH);
    
    printf("%-*s", ADDRESS_FIELD_LENGTH, "");
    for(int i = 0; i < BYTE_IN_ROW; i++)
    {
        printf("%-*x", BYTE_FIELD_LENGTH, i);
    }
    printf("\n");

    for(int i = 0; i < header->SizeOfRawData; i++)
    {
        if(i % BYTE_IN_ROW == 0)
        {
            if(i != 0) printf("\n");
            printf("%08x%*s", i, ADDRESS_FIELD_LENGTH - 8, "");
        }

        printf("%02x%*s", (unsigned char)content[i], BYTE_FIELD_LENGTH - 2, "");
    }

    printf("\n\n");
    
    free(title);
    title = NULL;
}

int main(int argc, char **argv)
{
    VerifyArg(argc, argv);
    const char *objFilePath = GetObjFilePath(argv);

    FILE *file = OpenObjectFile(objFilePath);

    // Read file header
    FILE_HEADER fileHeader;
    ReadFileHeader(file, &fileHeader);
    
    // advance the file cursor
    fseek(file, fileHeader.SizeOfOptionalHeader, SEEK_CUR);

    // read sections headers
    unsigned short numberOfSections = fileHeader.NumberOfSections;
    SECTION_HEADER *sectionHeaders = (SECTION_HEADER *) malloc(numberOfSections * sizeof(SECTION_HEADER));
    ReadSectionHeaders(file, sectionHeaders, numberOfSections);

    unsigned int stringTableOffset = fileHeader.PointerToSymbolTable + (fileHeader.NumberOfSymbols * 18);
    char *stringTable = ReadStringTable(file, stringTableOffset);
    
    char command[256] = {0};
    // compare first 4 character because fgets read newline too.
    while(!compareStrings(command, "exit", 4))
    {
        printf("List of commands: list, show, exit, help.\nUse help to get information about usage.\ncommand> ");
        fflush(stdout);
        
        if(!fgets(command, sizeof(command), stdin))
        {
            ShowError("Something went wrong reading input", "");
            continue;
        }
        
        char **commandList = divideCommand(command, ' ');
        // Command can be maximum consist of 4 part. Remove  newline character fgets introduce.
        for(int i = 0; i < 4; i++)
        {
            if(commandList[i])
            {
                char *commandPart = commandList[i];
                ReplaceCharacter(commandPart, '\n', '\0');
                ReplaceCharacter(commandPart, '\r', '\0');
            }
        }
        
        if(!commandList[0])
        {
            ShowError("Something went wrong reading input", "");
            continue;            
        }

        if(compareStrings(commandList[0], "exit", 4))
        {
            exit(0);
        }

        // printf("Result of commandList[0]: %s \n", commandList[0]);
        // printf("Result of compareString: %d \n", compareStrings(commandList[0], "list"));
        if (compareStrings(commandList[0], "list", 4))
        {
            if(!commandList[1])
            {
                ShowError("Invalid command format. Please see help command.\n", "");
                continue;
            }
            
            if (compareStrings(commandList[1], "sections", 8))
            {
                for(int i = 0; i < numberOfSections; i++)
                {
                    // This could be problematic for PE format because Name is not \0 terminated
                    // This is working by accident now because next byte coming after name is \0 by
                    // chance.
                    printf("%s \n", sectionHeaders[i].Name);
                }
                printf("\n");
                
            }else if (compareStrings(commandList[1], "headers", 7))
            {                
                PrintFileHeader(&fileHeader);
                PrintSectionHeaders(sectionHeaders, numberOfSections);
                
            }else if (compareStrings(commandList[1], "relocations", 11))
            {
                for(int i = 0; i < numberOfSections; i++){
                    
                    RELOCATION *relocationTable = ReadRelocationTable(file, sectionHeaders[i].Name, sectionHeaders, numberOfSections);
                    if(relocationTable)
                    {
                        PrintRelocations(&sectionHeaders[i], relocationTable);
                    }
                    free(relocationTable);
                    relocationTable = NULL;
                }
                
            }else if (compareStrings(commandList[1], "symbols", 7))
            {
                

                SYMBOL* symbols = ReadSymbols(file, fileHeader.PointerToSymbolTable, fileHeader.NumberOfSymbols);
                if(symbols)
                {

                    PrintSymbols(symbols, fileHeader.NumberOfSymbols, stringTable);
                    
                }
                free(symbols);
                symbols = NULL;
            }

            
        }else if (compareStrings(commandList[0], "show", 4))
        {
            if(!commandList[1])
            {
                ShowError("Invalid command format. Please see help command.\n", "");
                continue;
            }
            
            if (compareStrings(commandList[1], "section", 7))
            {
                if(!commandList[2])
                {
                    printf("Please provide section name. You can see useable section names with command: list sections.");
                    continue;
                }
                
                // Read header
                SECTION_HEADER* header = findSectionHeader(commandList[2], sectionHeaders, numberOfSections);
                if(!header)
                {
                    ShowError("Section header couldn't found. Use 'list sections' to get available section names.\n\n", "");
                    continue;
                }

                char *sectionContent = NULL;
                if(compareStrings(commandList[2], ".drectve", 8))
                {
                    sectionContent = ReadSectionText(file, header);
                    printf("Directive Section: %s\n\n", sectionContent);
                }else
                {
                    sectionContent = ReadSectionBinary(file, header);
                    PrintSectionBinary(sectionContent, header);
                }

                free(sectionContent);
                sectionContent = NULL;
            }
        }else if (compareStrings(commandList[0], "help", 4))
        {
            printf("list sections        --> lists all section names.\n");
            printf("list headers         --> lists all headers -file header, section headers-\n");
            printf("list relocations     --> list all relocations for all sections\n");
            printf("list symbols         --> lists symbol table.\n");

            printf("show section <sectionName>      --> shows content of the section\n");
            
            printf("exit    --> da\n");
            printf("help    --> da\n");
            printf("\n");

        }else
        {
            printf("Unknown command.\n");
        }
        
        free(commandList);
        commandList = NULL;

    }



    // Clear heap pointers

    free(stringTable);
    stringTable = NULL;
    
    free(sectionHeaders);
    sectionHeaders = NULL;

    
    fclose(file);
    return 0;
}
