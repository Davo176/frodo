
//
//  PQCgenKAT_kem.c
//
//  Modified from a file created by Bassham, Lawrence E (Fed) on 8/29/17.
//  Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rng.h"
#include "../src/api_frodo640.h"


#define	MAX_MARKER_LEN		50
#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

char    AlgName[] = "FrodoKEM-640-SHAKE";

int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);
void	fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L);

int
main()
{
    char                fn_req[32], fn_rsp[32];
    FILE                *fp_req, *fp_rsp;
    unsigned char       seed[48];
    unsigned char       entropy_input[48];
    unsigned char       ct[CRYPTO_CIPHERTEXTBYTES], ss[CRYPTO_BYTES], ss1[CRYPTO_BYTES];
    int                 count;
    int                 done;
    unsigned char       pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    int                 ret_val;

    // Create the REQUEST file
    sprintf(fn_req, "addDecapsTest_%d.req", CRYPTO_SECRETKEYBYTES);
    if ( (fp_req = fopen(fn_req, "w")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }
    sprintf(fn_rsp, "addDecapsTest_%d.rsp", CRYPTO_SECRETKEYBYTES);
    if ( (fp_rsp = fopen(fn_rsp, "w")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_rsp);
        return KAT_FILE_OPEN_ERROR;
    }

    unsigned char skUnderTest[10][CRYPTO_SECRETKEYBYTES];
    unsigned char ctUnderTest[10][CRYPTO_CIPHERTEXTBYTES];

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[0][i]=0;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[0][i]=0;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[1][i]=0;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[1][i]=255;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[2][i]=255;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[2][i]=0;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[3][i]=255;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[3][i]=255;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[4][i]=0;
        if (i==0){
            ctUnderTest[4][i]=255;
        }
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[4][i]=0;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[5][i]=0;
        if (i==CRYPTO_CIPHERTEXTBYTES-1){
            ctUnderTest[5][i]=255;
        }
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[5][i]=0;
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[6][i]=0;
        if (i==0){
            ctUnderTest[6][i]=255;
        }
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[6][i]=0;
        if (i==0){
            skUnderTest[6][i]=255;
        }
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[7][i]=0;
        if (i==CRYPTO_CIPHERTEXTBYTES-1){
            ctUnderTest[7][i]=255;
        }
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[7][i]=0;
        if (i==CRYPTO_SECRETKEYBYTES-1){
            skUnderTest[7][i]=255;
        }
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[8][i]=0;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[8][i]=0;
        if (i==0){
            skUnderTest[8][i]=255;
        }
    }

    for (int i=0;i<CRYPTO_CIPHERTEXTBYTES;i++){
        ctUnderTest[9][i]=0;
    }
    for (int i=0;i<CRYPTO_SECRETKEYBYTES;i++){
        skUnderTest[9][i]=0;
        if (i==CRYPTO_SECRETKEYBYTES-1){
            skUnderTest[9][i]=255;
        }
    }

    for (int i=0; i<48; i++)
        entropy_input[i] = i;

    randombytes_init(entropy_input, "WILL DAVIS", 256);
    for (int i=0; i<10; i++) {
        fprintf(fp_req, "count = %d\n", i);
        randombytes(seed, 48);
        fprintBstr(fp_req, "seed = ", seed, 48);
        fprintf(fp_req, "sk =\n");
        fprintf(fp_req, "ct =\n");
        fprintf(fp_req, "ss =\n\n");
    }
    fclose(fp_req);

    //Create the RESPONSE file based on what's in the REQUEST file
    if ( (fp_req = fopen(fn_req, "r")) == NULL ) {
        printf("Couldn't open <%s> for read\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }

    fprintf(fp_rsp, "# %s\n\n", AlgName);
    done = 0;
    do {
        if ( FindMarker(fp_req, "count = ") )
            fscanf(fp_req, "%d", &count);
        else {
            done = 1;
            break;
        }
        fprintf(fp_rsp, "count = %d\n", count);

        if ( !ReadHex(fp_req, seed, 48, "seed = ") ) {
            printf("ERROR: unable to read 'seed' from <%s>\n", fn_req);
            return KAT_DATA_ERROR;
        }
        fprintBstr(fp_rsp, "seed = ", seed, 48);

        randombytes_init(seed, NULL, 256);

        fprintBstr(fp_rsp, "sk = ", skUnderTest[count], CRYPTO_SECRETKEYBYTES);

        fprintBstr(fp_rsp, "ct = ", ctUnderTest[count], CRYPTO_CIPHERTEXTBYTES);

        fprintf(fp_rsp, "\n");

        if ( (ret_val = crypto_kem_dec_Frodo640(ss, ctUnderTest[count], skUnderTest[count])) != 0) {
            printf("crypto_kem_dec returned <%d>\n", ret_val);
            return KAT_CRYPTO_FAILURE;
        }

        fprintBstr(fp_rsp, "ss = ", ss, CRYPTO_BYTES);

    } while ( !done );

    fclose(fp_req);
    fclose(fp_rsp);

    return KAT_SUCCESS;
}

//
// ALLOW TO READ HEXADECIMAL ENTRY (KEYS, DATA, TEXT, etc.)
//
int
FindMarker(FILE *infile, const char *marker)
{
	char	line[MAX_MARKER_LEN];
	int		i, len;
	int curr_line;

	len = (int)strlen(marker);
	if ( len > MAX_MARKER_LEN-1 )
		len = MAX_MARKER_LEN-1;

	for ( i=0; i<len; i++ )
	  {
	    curr_line = fgetc(infile);
	    line[i] = curr_line;
	    if (curr_line == EOF )
	      return 0;
	  }
	line[len] = '\0';

	while ( 1 ) {
		if ( !strncmp(line, marker, len) )
			return 1;

		for ( i=0; i<len-1; i++ )
			line[i] = line[i+1];
		curr_line = fgetc(infile);
		line[len-1] = curr_line;
		if (curr_line == EOF )
		    return 0;
		line[len] = '\0';
	}

	// shouldn't get here
	return 0;
}

//
// ALLOW TO READ HEXADECIMAL ENTRY (KEYS, DATA, TEXT, etc.)
//
int
ReadHex(FILE *infile, unsigned char *A, int Length, char *str)
{
	int			i, ch, started;
	unsigned char	ich;

	if ( Length == 0 ) {
		A[0] = 0x00;
		return 1;
	}
	memset(A, 0x00, Length);
	started = 0;
	if ( FindMarker(infile, str) )
		while ( (ch = fgetc(infile)) != EOF ) {
			if ( !isxdigit(ch) ) {
				if ( !started ) {
					if ( ch == '\n' )
						break;
					else
						continue;
				}
				else
					break;
			}
			started = 1;
			if ( (ch >= '0') && (ch <= '9') )
				ich = ch - '0';
			else if ( (ch >= 'A') && (ch <= 'F') )
				ich = ch - 'A' + 10;
			else if ( (ch >= 'a') && (ch <= 'f') )
				ich = ch - 'a' + 10;
            else // shouldn't ever get here
                ich = 0;

			for ( i=0; i<Length-1; i++ )
				A[i] = (A[i] << 4) | (A[i+1] >> 4);
			A[Length-1] = (A[Length-1] << 4) | ich;
		}
	else
		return 0;

	return 1;
}

void
fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L)
{
	unsigned long long  i;

	fprintf(fp, "%s", S);

	for ( i=0; i<L; i++ )
		fprintf(fp, "%02X", A[i]);

	if ( L == 0 )
		fprintf(fp, "00");

	fprintf(fp, "\n");
}
