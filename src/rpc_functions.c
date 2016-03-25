/***********************************************************************
 * Permission is hereby granted to any individual or institution       *
 * for use, copying, or redistribution of the AV2XGobi C code          *
 * and associated documentation, provided such code and documentation  *
 * are not sold for profit and the following copyright notice is       *
 * retained in the code and documentation:                             *
 *                                                                     *
 *   Copyright (c) 1995 Iowa State University                          *
 *                                                                     *
 * We encourage you to share questions, comments and modifications.    *
 *                                                                     *
 *   Juergen Symanzik (symanzik@iastate.edu)                           *
 *   Dianne Cook (dicook@iastate.edu)                                  *
 *   James J. Majure (majure@iastate.edu)                              *
 *                                                                     *
 * The code in this file is based on the file functions.c              *
 * provided by ESRI.                                                   *
 * It has been modified to provide the functionality required for      *
 * the AV2XGOBI link.                                                  *
 * For completeness, we maintain the original Copyright notice:        *
 *                                                                     *
 ***********************************************************************/

/* @(#)functions.c      1.1 4/28/94 10:19:53
*
*-----------------------------------------------------------------------
*
*HB {Manual} {001011} {IAC}
*
*-----------------------------------------------------------------------
*
*N  Stubs for server functions. 
*
*-----------------------------------------------------------------------
*
*P  Purpose:
*
*   This file contains the stubs for server functions. These functions
*   are invoked from aiserver_svc_proc.c. These functions simply transfer
*   control to a user written function, where they are defined, and return
*   a "Procedure not defined" string otherwise. This is the file to be modified
*   if you want to add your own functions. 
*
*E
*
*-----------------------------------------------------------------------
*
*A  Arguments:
*
*   {{instr,outstr}}
*
*   {instr    <Input>    === (char *) Input string argument, if any}
*   {outstr   <Output>   === (char *) Results of the operation}
*
*E
*
*-----------------------------------------------------------------------
*
*H  History:
*
*    Ravi Narasimhan          [4/27/94]          Original coding.
*E
*HE
*-----------------------------------------------------------------------
*/

#ifdef RPC_USED

#include <string.h>

void Function1 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Colnames (instr,outstr);
}
void Function2 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Rownames (instr,outstr);
}
void Function3 (instr,outstr)
char *instr, *outstr;
{
	RPC_Clone_XGobi (instr,outstr);
}
void Function4 (instr,outstr)
char *instr, *outstr;
{
	RPC_Xfer_Colornames (instr,outstr);
}
void Function5 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 05 not implemented");
}
void Function6 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 06 not implemented");
}
void Function7 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 07 not implemented");
}
void Function8 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 08 not implemented");
}
void Function9 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 09 not implemented");
}
void Function10 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 10 not implemented");
}
void Function11 (instr,outstr)
char *instr, *outstr;
{
	RPC_Init_Data (instr,outstr);
}
void Function12 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_Data (instr,outstr);
}
void Function13 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_Symbols (instr,outstr);
}
void Function14 (instr,outstr)
char *instr, *outstr;
{
	RPC_Make_XGobi (instr,outstr);
}
void Function15 (instr,outstr)
char *instr, *outstr;
{
	RPC_Update_All_Symbols (instr,outstr);
}
void Function16 (instr,outstr)
char *instr, *outstr;
{
	RPC_Update_Some_Symbols (instr,outstr);
}
void Function17 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 17 not implemented");
}
void Function18 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 18 not implemented");
}
void Function19 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 19 not implemented");
}
void Function20 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 20 not implemented");
}
void Function21 (instr,outstr)
char *instr, *outstr;
{
	RPC_Init_CDF_Data (instr,outstr);
}
void Function22 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_CDF_Data (instr,outstr);
}
void Function23 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_CDF_Symbols (instr,outstr);
}
void Function24 (instr,outstr)
char *instr, *outstr;
{
	RPC_Make_CDF_XGobi (instr,outstr);
}
void Function25 (instr,outstr)
char *instr, *outstr;
{
	RPC_Add_CDF_Bitmap (instr,outstr);
}
void Function26 (instr,outstr)
char *instr, *outstr;
{
	RPC_Add_CDF_Type (instr,outstr);
}
void Function27 (instr,outstr)
char *instr, *outstr;
{
	RPC_Delete_CDF_Bitmap_and_Type (instr,outstr);
}
void Function28 (instr,outstr)
char *instr, *outstr;
{
	RPC_Brush_All_CDF_Symbols (instr,outstr);
}
void Function29 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_CDF_Weights (instr,outstr);
}
void Function30 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 30 not implemented");
}
void Function31 (instr,outstr)
char *instr, *outstr;
{
	RPC_Init_VARIO_Data (instr,outstr);
}
void Function32 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_VARIO_Data (instr,outstr);
}
void Function33 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_Init_VARIO_Symbols (instr,outstr);
}
void Function34 (instr,outstr)
char *instr, *outstr;
{
	RPC_Make_VARIO_XGobi (instr,outstr);
}
void Function35 (instr,outstr)
char *instr, *outstr;
{
	RPC_Update_Some_VARIO_Symbols (instr,outstr);
}
void Function36 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 36 not implemented");
}
void Function37 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 37 not implemented");
}
void Function38 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 38 not implemented");
}
void Function39 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 39 not implemented");
}
void Function40 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 40 not implemented");
}
void Function41 (instr,outstr)
char *instr, *outstr;
{
        RPC_Init_LAG_Data (instr,outstr);
}
void Function42 (instr,outstr)
char *instr, *outstr;
{
        RPC_Send_Init_LAG_Data (instr,outstr);
}
void Function43 (instr,outstr)
char *instr, *outstr;
{
        RPC_Send_Init_LAG_Symbols (instr,outstr);
}
void Function44 (instr,outstr)
char *instr, *outstr;
{
        RPC_Make_LAG_XGobi (instr,outstr);
}
void Function45 (instr,outstr)
char *instr, *outstr;
{
        RPC_Update_Some_LAG_Symbols (instr,outstr);
}
void Function46 (instr,outstr)
char *instr, *outstr;
{
	RPC_Send_LAG_Colnames (instr,outstr);
}
void Function47 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 47 not implemented");
}
void Function48 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 48 not implemented");
}
void Function49 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 49 not implemented");
}
void Function50 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 50 not implemented");
}
void Function51 (instr,outstr)
char *instr, *outstr;
{
        RPC_Init_VARIO2_Data (instr,outstr);
}
void Function52 (instr,outstr)
char *instr, *outstr;
{
        RPC_Send_Init_VARIO2_Data (instr,outstr);
}
void Function53 (instr,outstr)
char *instr, *outstr;
{
        RPC_Send_Init_VARIO2_Symbols (instr,outstr);
}
void Function54 (instr,outstr)
char *instr, *outstr;
{
        RPC_Make_VARIO2_XGobi (instr,outstr);
}
void Function55 (instr,outstr)
char *instr, *outstr;
{
        RPC_Update_Some_VARIO2_Symbols (instr,outstr);
}
void Function56 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 56 not implemented");
}
void Function57 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 57 not implemented");
}
void Function58 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 58 not implemented");
}
void Function59 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 59 not implemented");
}
void Function60 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 60 not implemented");
}
void Function61 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 61 not implemented");
}
void Function62 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 62 not implemented");
}
void Function63 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 63 not implemented");
}
void Function64 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 64 not implemented");
}
void Function65 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 65 not implemented");
}
void Function66 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 66 not implemented");
}
void Function67 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 67 not implemented");
}
void Function68 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 68 not implemented");
}
void Function69 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 69 not implemented");
}
void Function70 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 70 not implemented");
}
void Function71 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 71 not implemented");
}
void Function72 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 72 not implemented");
}
void Function73 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 73 not implemented");
}
void Function74 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 74 not implemented");
}
void Function75 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 75 not implemented");
}
void Function76 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 76 not implemented");
}
void Function77 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 77 not implemented");
}
void Function78 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 78 not implemented");
}
void Function79 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 79 not implemented");
}
void Function80 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 80 not implemented");
}
void Function81 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 81 not implemented");
}
void Function82 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 82 not implemented");
}
void Function83 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 83 not implemented");
}
void Function84 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 84 not implemented");
}
void Function85 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 85 not implemented");
}
void Function86 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 86 not implemented");
}
void Function87 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 87 not implemented");
}
void Function88 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 88 not implemented");
}
void Function89 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 89 not implemented");
}
void Function90 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 90 not implemented");
}
void Function91 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 91 not implemented");
}
void Function92 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 92 not implemented");
}
void Function93 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 93 not implemented");
}
void Function94 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 94 not implemented");
}
void Function95 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 95 not implemented");
}
void Function96 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 96 not implemented");
}
void Function97 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 97 not implemented");
}
void Function98 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 98 not implemented");
}
void Function99 (instr,outstr)
char *instr, *outstr;
{
	strcpy (outstr,"XGobi: Procedure 99 not implemented");
}

#endif
