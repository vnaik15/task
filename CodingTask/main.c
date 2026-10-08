/*****
    Task: Description: The sensor I am using generates a lot of data and there seems to be a lot of repeating sequences in it.
    The top bit is never used in the data format and could be re-purposed for whatever you like.

    The format looks like:
    0x70, 0x71, 0x34, 0x34, 0x34, 0x20, 0x67, 0x15, 0x15, 0x15, 0x15, 0x15, 0x42, 0x53, 0x68, 0x68, 0x68

    This is just one example of the format and there are many other sequences and combinations of the data that you should test.
    Design me a program that can store this data in the least number of bytes possible to a file, but that can also re-generate the original data set if needed.

    Author: Vatsal Naik
    Date: August 11 2021
*****/

#include <stdio.h>
#include <stdlib.h>

/** \brief
 *  This input data buffer can be fixed size.. eg. 8bytes and that can be receive through DMA.
    we can call DMA Half RX interrupt to process 4 Bytes of received data.
    in the mean time DMA will continue to receive remaining 4 bytes of data.
    By the time DMA Complete receive interrupt occcure, we can process remainign higher
    byte while dma will receive lower 4 bytes into a circular buffer.
 */

void NormalRW (void);     // This is just simple way to store data as an array.

int main()
{
    /**
    This approach will reduce size in the File, by using Top bit for the data...

    I have used top bit as a flag, to see next number in the array is same.
    No:-> Reset top bit, and move to next
    Yes:-> Set top bit, delete next number. and move to next.
    Example:
    {0x70, 0x71, 0x34, 0x34, 0x34, 0x20, 0x67, 0x15, 0x15, 0x15, 0x15, 0x15, 0x42, 0x53, 0x68, 0x68, 0x68}
    Above data will become
    {0x70, 0x71, 0xB4, 0x34, 0x20, 0x67, 0x95, 0x95, 0x15, 0x42, 0x53, 0xE8, 0x68};
    **/

    char *buffer, *values;
    int filelen;
    FILE *fileptr = NULL;
    int i = 0, j = 0;

    char input[12] = {0x70, 0x71, 0x34, 0x34, 0x34, 0x77, 0x77, 0x11, 0x00, 0x35, 0x15, 0x15 };
    int inputSize = sizeof(input)/sizeof(char);

    /// Encode / Compress Data.
    // 1. Find new Array size
    int newInputsize = 0;
    for (i = 0; i < inputSize; i++)
    {
        newInputsize++;
        if ((i + 1 < inputSize) && (input[i] == input[i+1]))
        {
            // Number match
            i++;
        }
    }
    //printf("UPDATED Data \n");
    //printf("New Data Size = %d\n", newInputsize);

    ///Dynamic Memory Allocation -> We can use LinkLists as well.
    values = (char *)calloc((size_t)newInputsize, sizeof(char));
    if (values == NULL)
    {
        fprintf(stderr, "Couldn't allocate compression buffer\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < inputSize; i++)
    {
        values[j] = input[i];
        if ((i + 1 < inputSize) && (input[i] == input[i+1]))
        {
            // Match -> Set top
            values[j] |= 0x80;
            i++;
        }
        else
        {
            // NoMatch -> Reset top bit
            values[j] &= ~0x80;
        }
        j++;
    }

    /// This For loop can be replace by DMA Interrupt handler or Different Thread...
    /*for (i = 0; i < newInputsize; i++)
    {
        values[i] = newInput[i];
    }*/

    // TODO: if more data needs to be added, use realloc to reassign a memory and continue
    /*for (i = 0; i < newInputsize; i++)
    {
        printf("0x%x\t", values[i]);
    }*/

    printf("Input DataLength = %d \t\tCompressed DataLength = %d\n", inputSize, newInputsize);

    /// Store Data to File
    fileptr = fopen("Data.bin", "wb");    // w for write, b for binary
    if (fileptr == NULL)
    {
        perror("Data.bin");
        free(values);
        return EXIT_FAILURE;
    }
    if (fwrite(values, sizeof(char), (size_t)newInputsize, fileptr) != (size_t)newInputsize)
    {
        perror("Data.bin");
        fclose(fileptr);
        free(values);
        return EXIT_FAILURE;
    }
    fclose(fileptr);

    /// TODO: if more data needs to be added, use realloc to reassign a memory and continue

    /// Read Data from File
    fileptr = fopen("Data.bin", "rb");    // r for read, b for binary
    if (fileptr == NULL)
    {
        perror("Data.bin");
        free(values);
        return EXIT_FAILURE;
    }
    fseek(fileptr, 0, SEEK_END);                   // Jump to the end of the file
    filelen = ftell(fileptr);                      // Get the current byte offset in the file
    rewind(fileptr);                               // Jump back to the beginning of the file

    buffer = (char *)calloc((size_t)filelen, sizeof(char));// Enough memory for the file
    if (buffer == NULL)
    {
        fprintf(stderr, "Couldn't allocate read buffer\n");
        fclose(fileptr);
        free(values);
        return EXIT_FAILURE;
    }
    if (fread(buffer, sizeof(char), (size_t)filelen, fileptr) != (size_t)filelen)
    {
        fprintf(stderr, "Failed to read complete compressed file\n");
        fclose(fileptr);
        free(buffer);
        free(values);
        return EXIT_FAILURE;
    }            // Read in the entire file
    fclose(fileptr);                               // Close the file

    /// Decode / decompress data
    // 1. Find new data size
    int newReadsize = 0;
    for (i = 0; i < filelen; i++)
    {
        newReadsize++;
        if (buffer[i] & 0x80)
        {
            newReadsize++;
        }
    }
    printf("Store DataLength = %d \t\tRecovered DataLength = %d\n", filelen, newReadsize);

    char Read[newReadsize];
    j = 0;
    for (i = 0; i < filelen; i++)
    {
        Read[j] = buffer[i];
        if (buffer[i] & 0x80)
        {
            Read[j++] &= ~0x80;
            Read[j] = Read[j-1];
        }
        j++;
    }

    // For Verification
    /*for (int i = 0; i < filelen; i++)
    {
        printf("0x%x\t", buffer[i]);
    }*/
    printf("\nRecovered Data\n");
    for (int i = 0; i < newReadsize; i++)
    {
        printf("0x%x\t", Read[i]);
    }

    // free memory when no longer needed.
    free (buffer);
    free (values);
    printf("\nDONE\n");

    return 0;
}

/**
This approach File size will depend on number of elements in Array.,
**/
void NormalRW (void)
{
    const int memSize = 17;
    char *buffer, *values;
    int filelen;
    FILE *fptr = NULL;

    char input[17] = {0x70, 0x71, 0x34, 0x34, 0x34, 0xB7, 0xB7, 0x15, 0x15, 0x15, 0x15, 0x15, 0x42, 0x53, 0x68, 0x68, 0x68};
    //char input[10] = {0x70, 0x71, 0x34, 0x34, 0x34, 0x20, 0x67, 0x15, 0x15, 0x15};

    // In PC size of int is 4 bytes
    //int Arraylength = sizeof(input)/sizeof(int);
    //values = (char *)malloc(memSize * sizeof(char));
    values = (char *)calloc(0, memSize * sizeof(char));

    if (values == NULL) /* Memory allocation fails */
    {
        printf("Couldn't allocate memory");

        return;
    }

    // This For loop can be replace by DMA Interrupt handler...
    for (int i = 0; i < memSize; i++)
    {
        values[i] = input[i];
    }

    // TODO: if more data needs to be added, use realloc to reassign a memory and continue

    // Store Data to File
    fptr = fopen("data.dat", "wb");             // write the file
    fwrite(values, memSize, sizeof(char), fptr);
    fclose(fptr);

    // Read Data from File
    fptr = fopen("data.dat", "rb");             // Open the file
    fseek(fptr, 0, SEEK_END);                   // Jump to the end of the file
    filelen = ftell(fptr);                      // Get the current byte offset in the file
    rewind(fptr);                               // Jump back to the beginning of the file

    // buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
    buffer = (char *)calloc(0, filelen * sizeof(char)); // Enough memory for the file
    if (buffer == NULL) /* Memory allocation fails */
    {
        printf("Couldn't allocate memory");
        return;
    }

    fread(buffer, filelen, 1, fptr);    // Read in the entire file
    fclose(fptr);                       // Close the file

    // For Verification
    for (int i = 0; i < filelen; i++)
    {
        printf("0x%x\n", buffer[i]);
    }

    free (buffer);                          // free memory when no longer needed.
    free (values);                              // free memory when no longer needed.
}

/*
void test (void)
{
    int *buffer, *values;
    int filelen;
    FILE *fptr = NULL;

    int input[6] = {0x70, 0x71, 0x34, 0x34, 0x34, 0x70};
    int inputSize = sizeof(input)/sizeof(int);

    // Encode / Compress Data.
    // 1. Find new Array size
    int newInputsize = 0;
    for (int i = 0; i < inputSize; i++)
    {
        newInputsize++;
        if ((i + 1 < inputSize) && (input[i] == input[i+1]))
        {
            // Number match
            i++;
        }
    }
    printf("UPDATED Data \n");
    printf("New Data Size = %d\n", newInputsize);

    int newInput[newInputsize];
    int j = 0;
    for (int i = 0; i < inputSize; i++)
    {
        newInput[j] = input[i];
        if ((i + 1 < inputSize) && (input[i] == input[i+1]))
        {
            // Match -> Set top
            newInput[j] |= 0x80;
            i++;
        }
        else
        {
            // NoMatch -> Reset top bit
            newInput[j] &= ~0x80;
        }
        j++;
    }

    // Dynamic Memory Allocation -> We can use LinkLists as well.
    values = (int *)calloc(0, newInputsize * sizeof(int));
    if (values == NULL) // Memory allocation fails
    {
        printf("Couldn't allocate memory");
        return;
    }
    // This For loop can be replace by DMA Interrupt handler...
    for (int i = 0; i < newInputsize; i++)
    {
        values[i] = newInput[i];
    }

    // TODO: if more data needs to be added, use realloc to reassign a memory and continue
    for (int i = 0; i < newInputsize; i++)
    {
        printf("0x%x\n", values[i]);
    }

    // Store Data to File
    //fptr = fopen("newData.dat", "wb");          // write the file
    fptr = fopen("newData.dat", "wb");          // write the file
    fwrite(values, newInputsize, 4, fptr);
    fclose(fptr);

    // Read Data from File
    fptr = fopen("newData.dat", "rb");          // Open the file
    fseek(fptr, 0, SEEK_END);                   // Jump to the end of the file
    filelen = ftell(fptr)/sizeof(int);                      // Get the current byte offset in the file
    rewind(fptr);                               // Jump back to the beginning of the file

    buffer = (int *)calloc(0, filelen); // Enough memory for the file
    if (buffer == NULL)                         // Memory allocation fails
    {
        printf("Couldn't allocate memory");
        return;
    }
    fread(buffer, filelen, 4, fptr);    // Read in the entire file

    fclose(fptr);                       // Close the file

    printf("Read From File \n");

    // For Verification
    for (int i = 0; i < filelen; i++)
    {
        printf("0x%x\n", buffer[i]);
    }

    free (values);                              // free memory when no longer needed.
    free (buffer);                          // free memory when no longer needed.
}
*/

