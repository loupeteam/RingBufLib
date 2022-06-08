/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: Alarm
 * File: BufferInit.st
 * Author: Josh
 * Created: December 07, 2012
 ********************************************************************
 * Implementation of program Alarm
 ********************************************************************/

#include "RingBufLib.h"

#if !defined(_SG4) || defined(_NOT_BR)
#include "bur.h"
#include "stdlib.h"
#define TMP_alloc(s, l) 0;*(l) = (UDINT)malloc(s)
#define TMP_free(s, l) 0;free((void*)l)
#endif

#include "string.h"

#define brsmemcpy(a,b,c) memcpy((void*)a,(void*)b,c)
#define brsmemset(a,b,c) memset((void*)a,b,c)
#define ERR_OK 0
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define limit(a,b,c) (max(a,min(b, c)))
#define DINT_TO_UDINT (unsigned long)
#define DINT_TO_UINT (unsigned short)
#define UINT_TO_DINT (signed long)
#define INT_TO_UDINT (unsigned long)
#define UINT_TO_INT (short)

unsigned short BufferInit(unsigned long Buffer, unsigned short MaxValues, unsigned long DataSize) {
	unsigned short returnValue=0;	
	Buffer_typ* ibuf = (Buffer_typ*)Buffer;
	if( BufferValid(Buffer) ) {
		BufferDestroy((UDINT)ibuf);	
	}
	if( ibuf != 0 ) {
		if( MaxValues > 0 ) {
			returnValue=TMP_alloc(MaxValues * DataSize, &ibuf->Data);
			if( returnValue == 0 ) {
				ibuf->DataSize=DataSize;
				ibuf->MaxValues=MaxValues;
				ibuf->TopIndex=0;
				ibuf->NumberValues=0;
			}
		}
	}

	return returnValue;
}

unsigned short BufferDestroy(unsigned long Buffer) {
	unsigned short returnValue = 0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		returnValue=TMP_free(ibuf->MaxValues * ibuf->DataSize ,ibuf->Data);
		ibuf->Data=0;
		ibuf->DataSize=0;
		ibuf->MaxValues=0;
		ibuf->TopIndex=-1;
		ibuf->NumberValues=0;
	} else {
		returnValue=	BufferStatus(Buffer);
	}
	return returnValue;
}

plcbit BufferValid(unsigned long Buffer) {
	plcbit returnValue=0;
	Buffer_typ* ibuf = (Buffer_typ*)Buffer;
	if( ibuf != 0 ) {
		if( ibuf->Data != 0 ) {
			if( ibuf->MaxValues > 0 ) {
				returnValue=1;
			}
		}	
	}
	return returnValue;
}	

unsigned short BufferStatus(unsigned long Buffer) {
	unsigned short returnValue = 0;
	Buffer_typ* ibuf = (Buffer_typ*)Buffer;
	if( ibuf != 0 ) {
		if( ibuf->Data != 0 ) {
			if( ibuf->MaxValues > 0 ) {
				returnValue=ERR_OK;
			} else {
				returnValue= DINT_TO_UINT(RING_BUF_ERR_MAX_VALUES_ZERO);				
			}
		} else {
			returnValue= DINT_TO_UINT(RING_BUF_ERR_DATA_NOT_INIT);
		}	
	} else {
		returnValue= DINT_TO_UINT(RING_BUF_ERR_INVALID_BUF_POINTER);
	}
	return returnValue;
}

plcbit BufferFull(unsigned long Buffer) {
	plcbit returnValue;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues < ibuf->MaxValues ) {
			returnValue=0;	
		} else {
			returnValue=1;				
		}
	} else {	
		returnValue=1;			
	}
	return returnValue;
}

unsigned short BufferClear(unsigned long Buffer) {
	unsigned short returnValue;
	if( BufferValid(Buffer) ) {
		returnValue=0;
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		brsmemset(ibuf->Data,0,ibuf->DataSize * ibuf->MaxValues);
		ibuf->TopIndex=0;
		ibuf->NumberValues=0;	
	} else {
		returnValue= BufferStatus(Buffer);
	}
	return returnValue;
}

unsigned short BufferRemoveBottom(unsigned long Buffer) {
	unsigned short returnValue=0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues > 0 ) {
			brsmemset( BufferGetItemAdr(Buffer,ibuf->NumberValues-1,0),0,ibuf->DataSize);
			ibuf->NumberValues=limit(0,ibuf->NumberValues-1,ibuf->MaxValues);				
		}	
	} else {
		returnValue= BufferStatus(Buffer);
	}
	return returnValue;
}

unsigned short BufferRemoveTop(unsigned long Buffer) {
	unsigned short returnValue=0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues > 0 ) {
			brsmemset( BufferGetItemAdr(Buffer,0,0),0,ibuf->DataSize);
			ibuf->TopIndex=ibuf->TopIndex+1;
			if( ibuf->TopIndex >= UINT_TO_INT(ibuf->MaxValues) ) {
				ibuf->TopIndex=0;
			}
			ibuf->NumberValues=limit(0,ibuf->NumberValues-1,ibuf->MaxValues);				
		}	
	} else {
		returnValue= BufferStatus(Buffer);
	}
	return returnValue;
}

unsigned short BufferRemoveOffset(unsigned long Buffer, unsigned short Offset, unsigned long Status) {
	int Index = 0;
	int CopyLen = 0; // Internal value for number of entries to copy for each operation
	unsigned short iNumEntries = 0; // Internal limited number of entries to copy
	unsigned long iDestination = 0; // Internal pointer to where to put the data for second copy
	unsigned short returnValue=0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues > 0 ) {
			if( Offset == 0 ) {
				BufferRemoveTop(Buffer);
			} else if( Offset == (ibuf->NumberValues-1) ) {
				BufferRemoveBottom(Buffer);
			} else if(  Offset < ibuf->NumberValues ) {
				Index=ibuf->TopIndex+Offset;
				//check for ring roll over
				if( Index >= UINT_TO_DINT(ibuf->MaxValues) ) {
					Index= Index - ibuf->MaxValues;
				}				
				//iNumEntries=limit(0,NumEntries,ibuf->NumberValues-Offset);
				//[00X0T00000]Offset == 8
				//[XT00000000]X TI == 4 index= 12
				//						index=2
				//[00T0X00000]
				//[0123456789]
				if( Index < ibuf->TopIndex ) {//offset roll over case
					//[00X0T0000]
					//[0123456789]
					//get number of indeces until 
					CopyLen= (ibuf->TopIndex-1)-Index;
					//shift data up to topindex
					brsmemcpy(BufferGetItemAdr(Buffer,Offset,0),BufferGetItemAdr(Buffer,Offset+1,0),INT_TO_UDINT(CopyLen)*ibuf->DataSize);
					//adjust number of values
					ibuf->NumberValues=limit(0,ibuf->NumberValues-1,ibuf->MaxValues);	
				} else {//offset not rolled over
					//[00TX000000]
					//[0123456789]
					//get number of indeces until highest index of the buffer (not bottom index)
					CopyLen=  ((ibuf->MaxValues-1) - Index);
					//shift data up to highest index
					brsmemcpy(BufferGetItemAdr(Buffer,Offset,0),BufferGetItemAdr(Buffer,Offset+1,0),INT_TO_UDINT(CopyLen)*ibuf->DataSize);
					if( ibuf->TopIndex > 0 ) {
						//shift index 0 to the top
						brsmemcpy(ibuf->Data+(ibuf->MaxValues-1)*ibuf->DataSize,ibuf->Data,ibuf->DataSize);
						if( ibuf->TopIndex > 1 ) {
							//shift data below top index
							CopyLen= (ibuf->TopIndex-1);
							brsmemcpy(ibuf->Data,ibuf->Data+ibuf->DataSize,INT_TO_UDINT(CopyLen)*ibuf->DataSize);
						};
					};
					//adjust number of values
					ibuf->NumberValues=limit(0,ibuf->NumberValues-1,ibuf->MaxValues);	
				};
				
				returnValue=iNumEntries;	
				
			};	
		};
	} else {
		returnValue= BufferStatus(Buffer);
	};
	return returnValue;
}

unsigned short BufferAddToTop(unsigned long Buffer, unsigned long Data) {
	unsigned short returnValue=0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		GetNextTopIndex(Buffer);
		brsmemcpy(ibuf->Data+ INT_TO_UDINT(ibuf->TopIndex) * ibuf->DataSize, Data ,ibuf->DataSize);
	} else {
		returnValue= BufferStatus(Buffer);
	}
	return returnValue;
}

unsigned short BufferAddToBottom(unsigned long Buffer, unsigned long Data) {
	unsigned short returnValue=0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		brsmemcpy(ibuf->Data + GetNextBottomIndex(Buffer) * ibuf->DataSize,Data,ibuf->DataSize);
	} else {
		returnValue=BufferStatus(Buffer);		
	}
	return returnValue;
}

unsigned long BufferGetItemAdr(unsigned long Buffer, unsigned short Offset, unsigned long Status) {
	unsigned long returnValue=0;
	int Index = 0;
	SetStatusPointer(Status,0);
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( Offset >= ibuf->NumberValues ) {
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_INDEX_OUTSIDE_RANGE));
		} else {	
			Index=ibuf->TopIndex+Offset;
			if( Index >= UINT_TO_DINT(ibuf->MaxValues) ) {
				Index= Index - ibuf->MaxValues;
			}	
			returnValue=ibuf->Data+INT_TO_UDINT(Index)*ibuf->DataSize;
		}
	} else {
		SetStatusPointer(Status,BufferStatus(Buffer));
	}
	return returnValue;
}

unsigned short BufferCopyItems(unsigned long Buffer, unsigned short Offset, unsigned short NumEntries, unsigned long Destination, unsigned long Status) {
	unsigned short returnValue=	0;	
	int Index = 0;
	int CopyLen = 0; // Internal value for number of entries to copy for each operation
	unsigned short iNumEntries = 0; // Internal limited number of entries to copy
	unsigned long iDestination = 0; // Internal pointer to where to put the data for second copy
	SetStatusPointer(Status,0);
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;

		if( Offset >= ibuf->NumberValues ) {
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_INDEX_OUTSIDE_RANGE));
		} else if( Destination == 0 ) {
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_DEST_INVALID));
		} else if( NumEntries == 0 ) {			
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_NUM_ENTRIES_ZERO));
		} else {	
			Index=ibuf->TopIndex+Offset;
			if( Index >= UINT_TO_DINT(ibuf->MaxValues) ) {
				Index= Index - ibuf->MaxValues;
			}				
			iNumEntries=limit(0,NumEntries,ibuf->NumberValues-Offset);
			CopyLen= limit(0,iNumEntries,ibuf->MaxValues-Index);
			brsmemcpy(Destination,ibuf->Data+INT_TO_UDINT(Index)*ibuf->DataSize,INT_TO_UDINT(CopyLen)*ibuf->DataSize);
			iDestination=Destination+INT_TO_UDINT(CopyLen)*ibuf->DataSize;
			if( CopyLen != UINT_TO_INT(iNumEntries) ) {
				CopyLen= (iNumEntries-CopyLen);
				brsmemcpy(iDestination,ibuf->Data,INT_TO_UDINT(CopyLen)*ibuf->DataSize);
			}
			returnValue=iNumEntries;
		}
	} else {
		SetStatusPointer(Status, BufferStatus(Buffer));
	}
	return returnValue;
}

unsigned short BufferBottom(unsigned long Buffer) {
	unsigned short returnValue=	0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues>0 ) {
			returnValue=ibuf->NumberValues-1;
		} else {
			returnValue=0;			
		}
	}
	return returnValue;
}
unsigned long GetNextTopIndex(unsigned long Buffer) {
	unsigned long returnValue= 0;
	Buffer_typ* ibuf;
	if( BufferValid(Buffer) ) {
		ibuf = (Buffer_typ*)Buffer;
		ibuf->TopIndex=ibuf->TopIndex-1;
		ibuf->NumberValues=limit(0,ibuf->NumberValues+1,ibuf->MaxValues);
		if( ibuf->TopIndex < 0 ) {
			ibuf->TopIndex=ibuf->MaxValues-1;
		}
	}
	returnValue=ibuf->TopIndex;
	return returnValue;
}

unsigned long GetNextBottomIndex(unsigned long Buffer) {
	unsigned long returnValue=	0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		ibuf->NumberValues=ibuf->NumberValues+1;
		if( ibuf->NumberValues > ibuf->MaxValues ) {
			ibuf->TopIndex=ibuf->TopIndex+1;
			if( ibuf->TopIndex >= UINT_TO_DINT(ibuf->MaxValues) ) {
				ibuf->TopIndex=0;	
			}				
		}
		ibuf->NumberValues=limit(0,ibuf->NumberValues,ibuf->MaxValues);
		returnValue=GetBottomIndex(Buffer);
	}
	return returnValue;
}

unsigned short GetBottomIndex(unsigned long Buffer) {
	unsigned short returnValue=	0;	
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		if( ibuf->NumberValues > 0 ) {
			returnValue=ibuf->TopIndex+ibuf->NumberValues-1;
			if( returnValue >= ibuf->MaxValues ) {
				returnValue=returnValue-ibuf->MaxValues;
			}
		} else {
			returnValue=0;			
		}			
	}
	return returnValue;
}

unsigned short GetTopIndex(unsigned long Buffer) {
	unsigned short returnValue=	0;
	if( BufferValid(Buffer) ) {
		Buffer_typ* ibuf = (Buffer_typ*)Buffer;
		returnValue=ibuf->TopIndex;		
	}
	return returnValue;
}

plcbit SetStatusPointer(unsigned long pStatus, unsigned short Status) {
	plcbit returnValue=0;
	unsigned short* iStatus;
	if( pStatus != 0 ) {
		returnValue=1;
		iStatus = pStatus;
		*iStatus = Status;
	}	
	return returnValue;
}			