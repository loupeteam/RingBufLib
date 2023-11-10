(*
 * File: RingBuf.st
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of RingBufLib, licensed under the MIT License.
 *
 ********************************************************************
 * Implementation of program Alarm
 ********************************************************************) 

(* TODO: Add your comment here *)
FUNCTION BufferInit
	BufferInit:=0;	
	ibuf ACCESS Buffer;
	IF BufferValid(Buffer) THEN
		BufferDestroy(ADR(ibuf));	
	END_IF
	IF ADR(ibuf) <> 0 THEN
		IF MaxValues > 0 THEN
			BufferInit:=TMP_alloc(MaxValues * DataSize,ADR(ibuf.Data));
			IF BufferInit = 0 THEN
				ibuf.DataSize:=DataSize;
				ibuf.MaxValues:=MaxValues;
				ibuf.TopIndex:=0;
				ibuf.NumberValues:=0;
			END_IF
		END_IF
	END_IF
END_FUNCTION

FUNCTION BufferDestroy
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		BufferDestroy:=TMP_free(ibuf.MaxValues * ibuf.DataSize ,ibuf.Data);
		ibuf.Data:=0;
		ibuf.TopIndex:=-1;
	ELSE
		BufferDestroy:=	BufferStatus(Buffer);
	END_IF
END_FUNCTION

FUNCTION BufferValid
	BufferValid:=0;
	ibuf ACCESS Buffer;
	IF ADR(ibuf) <> 0 THEN
		IF ibuf.Data <> 0 THEN
			IF ibuf.MaxValues > 0 THEN
				BufferValid:=1;
			END_IF
		END_IF	
	END_IF
END_FUNCTION	

FUNCTION BufferStatus
	ibuf ACCESS Buffer;
	IF ADR(ibuf) <> 0 THEN
		IF ibuf.Data <> 0 THEN
			IF ibuf.MaxValues > 0 THEN
				BufferStatus:=ERR_OK;
			ELSE
				BufferStatus:= DINT_TO_UINT(RING_BUF_ERR_MAX_VALUES_ZERO);				
			END_IF
		ELSE
			BufferStatus:= DINT_TO_UINT(RING_BUF_ERR_DATA_NOT_INIT);
		END_IF	
	ELSE
		BufferStatus:= DINT_TO_UINT(RING_BUF_ERR_INVALID_BUF_POINTER);
	END_IF
END_FUNCTION

FUNCTION BufferFull
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues < ibuf.MaxValues THEN
			BufferFull:=0;	
		ELSE
			BufferFull:=1;				
		END_IF
	ELSE	
		BufferFull:=1;			
	END_IF	
END_FUNCTION

FUNCTION BufferClear
	IF BufferValid(Buffer) THEN
		BufferClear:=0;
		ibuf ACCESS Buffer;
		brsmemset(ibuf.Data,0,ibuf.DataSize * ibuf.MaxValues);
		ibuf.TopIndex:=0;
		ibuf.NumberValues:=0;	
	ELSE
		BufferClear:= BufferStatus(Buffer);
	END_IF	
END_FUNCTION

FUNCTION BufferRemoveBottom
	BufferRemoveBottom:=0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues > 0 THEN
			brsmemset( BufferGetItemAdr(Buffer,ibuf.NumberValues-1,0),0,ibuf.DataSize);
			ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues-1,ibuf.MaxValues);				
		END_IF	
	ELSE
		BufferRemoveBottom:= BufferStatus(Buffer);
	END_IF	
END_FUNCTION

FUNCTION BufferRemoveTop
	BufferRemoveTop:=0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues > 0 THEN
			brsmemset( BufferGetItemAdr(Buffer,0,0),0,ibuf.DataSize);
			ibuf.TopIndex:=ibuf.TopIndex+1;
			IF ibuf.TopIndex >= UINT_TO_INT(ibuf.MaxValues) THEN
				ibuf.TopIndex:=0;
			END_IF
			ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues-1,ibuf.MaxValues);				
		END_IF	
	ELSE
		BufferRemoveTop:= BufferStatus(Buffer);
	END_IF	
END_FUNCTION

FUNCTION BufferRemoveOffset
	BufferRemoveOffset:=0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues > 0 THEN
			IF Offset = 0 THEN
				BufferRemoveTop(Buffer);
			ELSIF Offset = (ibuf.NumberValues-1) THEN
				BufferRemoveBottom(Buffer);
			ELSIF  Offset < ibuf.NumberValues THEN
				Index:=ibuf.TopIndex+Offset;
				//check for ring roll over
				IF Index >= UINT_TO_DINT(ibuf.MaxValues) THEN
					Index:= Index - ibuf.MaxValues;
				END_IF				
				//iNumEntries:=LIMIT(0,NumEntries,ibuf.NumberValues-Offset);
				//[00X0T00000]Offset = 8
				//[XT00000000]X TI = 4 index= 12
				//						index=2
				//[00T0X00000]
				//[0123456789]
				IF Index < ibuf.TopIndex THEN//offset roll over case
					//[00X0T0000]
					//[0123456789]
					//get number of indeces until 
					CopyLen:= (ibuf.TopIndex-1)-Index;
					//shift data up to topindex
					brsmemcpy(BufferGetItemAdr(Buffer,Offset,0),BufferGetItemAdr(Buffer,Offset+1,0),INT_TO_UDINT(CopyLen)*ibuf.DataSize);
					//adjust number of values
					ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues-1,ibuf.MaxValues);	
				ELSE//offset not rolled over
					//[00TX000000]
					//[0123456789]
					//get number of indeces until highest index of the buffer (not bottom index)
					CopyLen:=  ((ibuf.MaxValues-1) - Index);
					//shift data up to highest index
					brsmemcpy(BufferGetItemAdr(Buffer,Offset,0),BufferGetItemAdr(Buffer,Offset+1,0),INT_TO_UDINT(CopyLen)*ibuf.DataSize);
					IF ibuf.TopIndex > 0 THEN
						//shift index 0 to the top
						brsmemcpy(ibuf.Data+(ibuf.MaxValues-1)*ibuf.DataSize,ibuf.Data,ibuf.DataSize);
						IF ibuf.TopIndex > 1 THEN
							//shift data below top index
							CopyLen:= (ibuf.TopIndex-1);
							brsmemcpy(ibuf.Data,ibuf.Data+ibuf.DataSize,INT_TO_UDINT(CopyLen)*ibuf.DataSize);
						END_IF;
					END_IF;
					//adjust number of values
					ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues-1,ibuf.MaxValues);	
				END_IF;
				
				BufferRemoveOffset:=iNumEntries;	
				
			END_IF;	
		END_IF;
	ELSE
		BufferRemoveOffset:= BufferStatus(Buffer);
	END_IF;
END_FUNCTION

FUNCTION BufferAddToTop
	BufferAddToTop:=0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		GetNextTopIndex(Buffer);
		brsmemcpy(ibuf.Data+ INT_TO_UDINT(ibuf.TopIndex) * ibuf.DataSize, Data ,ibuf.DataSize);
	ELSE
		BufferAddToTop:= BufferStatus(Buffer);
	END_IF
END_FUNCTION

FUNCTION BufferAddToBottom
	BufferAddToBottom:=0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		brsmemcpy(ibuf.Data + GetNextBottomIndex(Buffer) * ibuf.DataSize,Data,ibuf.DataSize);
	ELSE
		BufferAddToBottom:=BufferStatus(Buffer);		
	END_IF
END_FUNCTION

FUNCTION BufferGetItemAdr
	BufferGetItemAdr:=0;
	SetStatusPointer(Status,0);
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF Offset >= ibuf.NumberValues THEN
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_INDEX_OUTSIDE_RANGE));
		ELSE	
			Index:=ibuf.TopIndex+Offset;
			IF Index >= UINT_TO_DINT(ibuf.MaxValues) THEN
				Index:= Index - ibuf.MaxValues;
			END_IF	
			BufferGetItemAdr:=ibuf.Data+INT_TO_UDINT(Index)*ibuf.DataSize;
		END_IF
	ELSE
		SetStatusPointer(Status,BufferStatus(Buffer));
	END_IF
END_FUNCTION

FUNCTION BufferCopyItems
	BufferCopyItems:=	0;	
	SetStatusPointer(Status,0);
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;

		IF Offset >= ibuf.NumberValues THEN
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_INDEX_OUTSIDE_RANGE));
		ELSIF Destination = 0 THEN
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_DEST_INVALID));
		ELSIF NumEntries = 0 THEN			
			SetStatusPointer(Status,DINT_TO_UINT(RING_BUF_ERR_NUM_ENTRIES_ZERO));
		ELSE	
			Index:=ibuf.TopIndex+Offset;
			IF Index >= UINT_TO_DINT(ibuf.MaxValues) THEN
				Index:= Index - ibuf.MaxValues;
			END_IF				
			iNumEntries:=LIMIT(0,NumEntries,ibuf.NumberValues-Offset);
			CopyLen:= LIMIT(0,iNumEntries,ibuf.MaxValues-Index);
			brsmemcpy(Destination,ibuf.Data+INT_TO_UDINT(Index)*ibuf.DataSize,INT_TO_UDINT(CopyLen)*ibuf.DataSize);
			iDestination:=Destination+INT_TO_UDINT(CopyLen)*ibuf.DataSize;
			IF CopyLen <> UINT_TO_INT(iNumEntries) THEN
				CopyLen:= (iNumEntries-CopyLen);
				brsmemcpy(iDestination,ibuf.Data,INT_TO_UDINT(CopyLen)*ibuf.DataSize);
			END_IF
			BufferCopyItems:=iNumEntries;
		END_IF
	ELSE
		SetStatusPointer(Status, BufferStatus(Buffer));
	END_IF	
END_FUNCTION

FUNCTION BufferBottom
	BufferBottom:=	0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues>0 THEN
			BufferBottom:=ibuf.NumberValues-1;
		ELSE
			BufferBottom:=0;			
		END_IF
	END_IF
END_FUNCTION
FUNCTION GetNextTopIndex
	GetNextTopIndex:= 0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		ibuf.TopIndex:=ibuf.TopIndex-1;
		ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues+1,ibuf.MaxValues);
		IF ibuf.TopIndex < 0 THEN
			ibuf.TopIndex:=ibuf.MaxValues-1;
		END_IF
	END_IF
	GetNextTopIndex:=ibuf.TopIndex;
END_FUNCTION

FUNCTION GetNextBottomIndex
	GetNextBottomIndex:=	0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		ibuf.NumberValues:=ibuf.NumberValues+1;
		IF ibuf.NumberValues > ibuf.MaxValues THEN
			ibuf.TopIndex:=ibuf.TopIndex+1;
			IF ibuf.TopIndex >= UINT_TO_DINT(ibuf.MaxValues) THEN
				ibuf.TopIndex:=0;	
			END_IF				
		END_IF
		ibuf.NumberValues:=LIMIT(0,ibuf.NumberValues,ibuf.MaxValues);
		GetNextBottomIndex:=GetBottomIndex(Buffer);
	END_IF
END_FUNCTION

FUNCTION GetBottomIndex
	GetBottomIndex:=	0;	
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		IF ibuf.NumberValues > 0 THEN
			GetBottomIndex:=ibuf.TopIndex+ibuf.NumberValues-1;
			IF GetBottomIndex >= ibuf.MaxValues THEN
				GetBottomIndex:=GetBottomIndex-ibuf.MaxValues;
			END_IF
		ELSE
			GetBottomIndex:=0;			
		END_IF			
	END_IF
END_FUNCTION

FUNCTION GetTopIndex
	GetTopIndex:=	0;
	IF BufferValid(Buffer) THEN
		ibuf ACCESS Buffer;
		GetTopIndex:=ibuf.TopIndex;		
	END_IF
END_FUNCTION

FUNCTION SetStatusPointer
	SetStatusPointer:=0;
	
	IF pStatus <> 0 THEN
		SetStatusPointer:=1;
		iStatus ACCESS pStatus;
		iStatus:= Status;
	END_IF			

END_FUNCTION			