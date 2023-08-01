#pragma once
#include "GMLVariable.h"

#pragma pack(push, 1)

struct screenPos
{
public:

	screenPos(float nx, float ny) {
		this->x = nx;
		this->y = ny;
		this->z = 0;
		this->empty1 = 0;
		this->empty2 = 0;
		this->empty3 = 0;
	}

	// 0x0000 - 0x0003
	float z;

	// 0x0004 - 0x0007
	float x;

	// 0x0008 - 0x000B
	float y;

	// 0x000C - 0x000F
	float empty1;

	// 0x0010 - 0x0013
	float empty2;

	// 0x0014 - 0x0017
	float empty3;
protected:
private:
};

class GameMaker_TextStructure
{
public:

	void Set(float nx, float ny, const char *p)
	{
		this->x = nx;
		this->y = ny;
		this->pszText = (char*)(p);
	}

	// 0x0000 - 0x0003
	float x;

	// 0x0004 - 0x0007
	float y;

	// 0x0008 - 0x000B
	char * pszText;
protected:
private:
};

class GameMaker_Event
{
public:

	// 0x0000 - 0x0003
	unsigned long m_dwUnk0000_0003;

	// 0x0004 - 0x0007
	unsigned long m_dwUnk0004_0007;

	// 0x0008 - 0x000B
	// StepType, I believe it was
	unsigned long m_dwEventType;

	// 0x000C - 0x004F
	unsigned char uc_Unk0000_004F[0x50 - 0x0C];

	// 0x0050 - 0x0053
	unsigned long m_dwCodePtr;

	// 0x0054 - 0x0057
	unsigned long m_dwUnk0054_0057;

	// 0x0058 - 0x005B
	unsigned long m_dwUnk0058_005B;

	// 0x005C - 0x005F
	char * m_pszScriptName;

	// 0x0060 - 0x0063
	unsigned long m_dwUnk0060_0063;

	// 0x0064 - 0x0067
	unsigned long m_dwCodePtr2;

protected:
private:
};

struct GMLObjectBindingData
{
public:

	// 0x0000 - 0x0003
	char * m_pszObjectName;
protected:
private:
};

class InternedVariableData {
public:
	// 0x0000 - 0x0003
	unsigned long m_ID;

	// 0x0004 - 0x0007
	const char* m_pcszName;

	// 0x0008 -  0x000B
	unsigned long m_NameHash;

};

class InstanceVariableInternal {
public:

	inline unsigned long get_raw() {
		return (unsigned long)(this);
	}
	// 0x0000 - 0x0003
	VariablePointer variables[0x500];
protected:
private:

};
class InstanceVariableProperties {
public:
	// 0x0000 - 0x0003
	unsigned long m_dwUnk0000_0003;

	// 0x0004 - 0x0007
	unsigned long m_nVariableCount;

	// 0x0008 - 0x000B
	unsigned long m_dwUnk0008_000B;

	// 0x000C - 0x000F
	unsigned long m_dwUnk000C_000F;

	// 0x0010 - 0x0013
	InstanceVariableInternal* m_pInternal;
protected:
private:
};



typedef struct t_GMLRoomObjectInstance
{

	// 0x00 - 0x03
	unsigned long m_dwUnknown00_03;

	// 0x04 - 0x2B
	unsigned char m_ucUnknown04_2B[0x2C - 0x04];
	/*
	// 0x2C 
	bool m_bDoesDraw;

	// 0x2D
	bool m_bDeactivated;

	// 0x2E
	bool m_bUnk002E;

	// 0x2F
	bool m_bUnk002F;*/

	// 0x002C - 0x002F
	InstanceVariableProperties* m_pInstProps;

	// 0x0030 - 0x0067
	unsigned char m_ucUnk0030_0077[0x68 - 0x30];

	// 0x0068 - 0x006B
	GMLObjectBindingData * m_pBindData;

	// 0x006C - 0x006F
	unsigned long m_dwUnk006C_006F;

	// 0x0070 - 0x0073
	unsigned long m_dwUnk0070_0073;

	// 0x0074 - 0x0077
	unsigned long m_dwUnk0074_0077;

	// 0x0078 - 0x007B
	unsigned long m_InstanceID;

	// 0x007C - 0x007F
	unsigned long m_dwUnk007C_007F;

	// 0x0080 - 0x009F
	unsigned char m_ucUnk0080_009F[0xA0 - 0x80];

	// 0x00A0 - 0x00A3
	float m_fX;

	// 0x00A4 - 0x00A7
	float m_fY;

	// 0x00A8 - 0x012F
	unsigned char m_ucUnk00A8_012F[0x130 - 0xA8];

	// 0x130 - 0x133
	t_GMLRoomObjectInstance * m_pNext;

} GMLRoomObjectInstance;
typedef struct t_YoYoList
{
	// 0x00 - 0x03
	unsigned long m_dwUnknown00_03;

	// 0x04 - 0x12F
	unsigned char m_ucUnknown00_013B[0x130 - 0x04];

	// 0x130 - 0x133
	GMLRoomObjectInstance * m_pObjectLList;

} YoYoList;

struct GMLRoom
{

	// 0x00 - 0x03
	unsigned long m_dwUnknown00_03;

	// 0x04 - 0x7F
	unsigned char m_ucUnknown04_7F[0x80 - 0x04];

	// 0x80 - 0x83
	YoYoList * m_pYoyoList;

	// 0x84 - 0x87
	unsigned long m_dwUnknown84_87;

	// 0x88 - 0x8B
	unsigned long m_ObjListCount;

};


class YYGSRawString
{
public:

	// 0x0000 - 0x0003
	char * m_pString[1];

	// 0x0004 - 0x0007
	unsigned int m_nArgumentType2;

	// 0x0008 - 0x000B
	unsigned int m_nLength;

	// 0x000C - 0x000F
	unsigned long pad4_1;

	// 0x0010 - 0x0110
	unsigned char pad100[0x100];
protected:
private:
};

class YYGS_RetValUnknown
{
public:
	unsigned char data[0x100];
protected:
private:
};


class YYGArray {

	__declspec(noinline) double* raw()
	{
		unsigned long a = this->temp;
		a = *(unsigned long*)(a + 0x04);
		a = *(unsigned long*)(a + 0x04);
		return (double*)(a);
	}

	unsigned int length()
	{
		unsigned long a = *(unsigned long*)(temp);
		return *(unsigned long*)(a + 0x84);
	}

	// 0x0000 - 0x0003
	unsigned long temp;

	// 0x004 - 0x0083;
	unsigned char uc_Unk[0x84 - 0x4];
};

class YYGS_RetValArray
{
public:

	__declspec(noinline) double * CastForRawAccess()
	{
		unsigned long a = this->temp;
		a = *(unsigned long*)(a + 0x04);
		a = *(unsigned long*)(a + 0x04);
		return (double*)(a);
	}

	// I think this returns length, or it's a major coincidence
	unsigned int GetLength()
	{
		unsigned long a = *(unsigned long*)(temp);
		return  *(unsigned long*)(a + 0x84);
	}

	// 0x0000 - 0x0003
	unsigned long temp;

	// 0x0004 - 0x0100
	unsigned char pad4_100[0x100 - 0x4];
protected:
private:
};

class YYGS_RetValDouble
{
public:

	// 0x0000 - 0x0007
	double m_Value;

	// 0x0008 - 0x000B
	unsigned long pad8_b;

	// 0x000C - 0x000F
	unsigned long padc_f;
protected:
private:
};



class _CString
{
public:

	_CString() {}

	// 0x0000 - 0x0003
	char * ppstr;

	// 0x0004 - 0x0007
	unsigned long pad4_7;

	// 0x0008 - 0x000B
	unsigned long m_nLength;

	// 0x000C - 0x000F
	unsigned int m_nArgumentType;
protected:
private:
};


class YYGString
{
public:

	
	YYGString() {
		this->pad4_7          = 0xAAAAAAAA;
		this->pad8_b          = 0xBBBBBBBB;
		this->p = new _CString();
	}

	~YYGString() {
		delete this->p;
	}

	YYGString * operator() (const char* value) {
		this->p->ppstr = (char*)value;
		this->p->m_nArgumentType = 0x01;
		this->p->m_nLength = strlen(value);
		this->m_nArgumentType = 0x01;
		return this;
	}

	void Set(const char * value)
	{
		this->p->ppstr = (char*)value;
		this->p->m_nArgumentType = 0x01;
		this->p->m_nLength = strlen(value);
		this->m_nArgumentType = 0x01;
	}
	// this is at 
	// 0x0000 - 0x0003
	//YYGSRawString * m_pRaw;
	_CString *p;

	// 0x0004 - 0x0007
	unsigned long pad4_7;

	// 0x0008 - 0x000B
	unsigned long pad8_b;

	// 0x000C - 0x000F
	unsigned int m_nArgumentType;

	// This is as far as it goes

	// 0x0010 - 0x0110
	//unsigned char pad10_110[0x100];
protected:
private:
};

class YYGArgumentRealNumber
{
public:
	// 0x0000 - 0x0007
	double m_Value;

	// 0x0008 - 0x000B
	unsigned long pad8_b;

	// 0x000C - 0x000F
	unsigned long padc_f;

protected:
private:
};

class YYGArgumentString
{
public:

	/*
	// 0x0000 - 0x0003
	char * ppstr;

	// 0x0004 - 0x0007
	unsigned long pad4_7;

	// 0x0008 - 0x000B
	unsigned long m_nLength;

	// 0x000C - 0x000F
	unsigned long m_nArgumentType;*/

	YYGArgumentString()
	{
		this->p = new _CString();
		this->p->m_nArgumentType = 0x01;
		this->p->ppstr = nullptr;
		this->p->pad4_7 = 0x0;
		this->p->m_nLength = 0;
	}

	//  0x0000 - 0x0003
	_CString *p;
protected:
private:
};


class YYGArgument
{
public:

	YYGArgument() { 
		memset(&this->m_Bytes[0], 0, sizeof(m_Bytes) / sizeof(m_Bytes[0])); 
	}

	void CreateStringArgument(const char * value)
	{
		memset(&this->m_Bytes[0], 0, sizeof(m_Bytes) / sizeof(m_Bytes[0]));
		char * pStringArgument = (char*)(&this->m_Bytes[0x00]);

		pStringArgument = (char*)value;

		unsigned long * length = (unsigned long*)(&this->m_Bytes[0x08]);
		*length = strlen(value);

		unsigned long * argumentType = (unsigned long*)(&this->m_Bytes[0x0C]);
		*argumentType = 0x01;
	}

	void CreateRealNumericArgument(double real)
	{
		memset(&this->m_Bytes[0], 0, sizeof(m_Bytes) / sizeof(m_Bytes[0]));
		double * pRealArgument = (double*)(&this->m_Bytes[0x00]);

		*pRealArgument = real;

		unsigned long * argumentType = (unsigned long*)(&this->m_Bytes[0x0C]);
		*argumentType = 0x00;
	}

	// 0x0000 - 0x000F
	unsigned char m_Bytes[0x0F];
protected:
private:
};

class YYGNumberAndStringArgument
{
public:

	YYGNumberAndStringArgument()
	{
		this->str = new YYGArgumentString();
	}
	void SetNumber(double value)
	{
		this->real.m_Value = value;
		this->real.pad8_b = 0x0;

		// set argument type
		this->real.padc_f = 0x00;
	}
	
	void SetString(const char * value)
	{
		this->str->p->ppstr = (char*)value;
		this->str->p->m_nLength = strlen(value);

		this->pad14_17 = 0x00;
		this->pad18_1b = 0x00;
		this->m_nArg2Type = 0x01;
	}

	// 0x0000 - 0x000F
	YYGArgumentRealNumber real;

	// 0x0010 - 0x0013
	YYGArgumentString *str;

	// 0x0014 - 0x0017
	unsigned long pad14_17;

	// 0x0018 - 0x001B
	unsigned long pad18_1b;

	// 0x001C - 0x001F
	unsigned long m_nArg2Type;
protected:
private:

};

class YYGArgumentList
{
public:

	YYGArgumentList() {
		this->m_pArg1 = new YYGArgument();
		this->m_pArg2 = new YYGArgument();
		this->m_pArg3 = new YYGArgument();

		this->pad4_7 = 0x0;
		this->pad8_b = 0x0;
		this->pad14_17 = 0x0;
		this->pad18_1b = 0x0;
		this->pad24_27 = 0x0;
		this->pad28_2b = 0x0;
		this->m_nArgumentType1 = 0x00;
		this->m_nArgumentType2 = 0x00;
		this->m_nArgumentType3 = 0x00;
	}

	void SetArgumentAsString(unsigned int argNum, const char * value)
	{
		switch (argNum)
		{
		case 1:
			this->m_nArgumentType1 = 0x01;
			this->m_pArg1->CreateStringArgument(value);
			break;
		case 2:
			this->m_nArgumentType2 = 0x01;
			this->m_pArg2->CreateStringArgument(value);
			break;
		case 3:
			this->m_nArgumentType3 = 0x01;
			this->m_pArg3->CreateStringArgument(value);
			break;
		default:
			break;
		}
	}

	void SetArgumentAsRealNumeric(unsigned int argNum, double value)
	{
		switch (argNum)
		{
		case 1:
			this->m_nArgumentType1 = 0x00;
			this->m_pArg1->CreateRealNumericArgument(value);
			break;
		case 2:
			this->m_nArgumentType2 = 0x00;
			this->m_pArg2->CreateRealNumericArgument(value);
			break;
		case 3:
			this->m_nArgumentType3 = 0x00;
			this->m_pArg3->CreateRealNumericArgument(value);
			break;
		default:
			break;
		}
	}

	// Every 16 bytes there's another argument.
	// So we should just account for 3 atm.

	// 0x0000 - 0x0003
	YYGArgument * m_pArg1;

	// 0x0004 - 0x0007
	unsigned long pad4_7;

	// 0x0008 - 0x000B
	unsigned long pad8_b;

	// 0x000C - 0x000F
	unsigned long m_nArgumentType1;



	// 0x0010 - 0x0013
	YYGArgument * m_pArg2;

	// 0x0014 - 0x0017
	unsigned long pad14_17;

	// 0x0018 - 0x001B
	unsigned long pad18_1b;

	// 0x001C - 0x001F
	unsigned long m_nArgumentType2;



	// 0x0020 - 0x0023
	YYGArgument * m_pArg3;
	
	// 0x0024 - 0x0027
	unsigned long pad24_27;

	// 0x0028 - 0x002B
	unsigned long pad28_2b;

	// 0x002C - 0x002F
	unsigned long m_nArgumentType3;
protected:
private:
};
#pragma pack(pop)
