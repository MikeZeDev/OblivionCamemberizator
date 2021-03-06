#pragma once

class TESForm;
class Script;

const char * GetObjectClassName(void * objBase);
void DumpClass(void * theClassPtr, UInt32 nIntsToDump = 512);

#ifdef OBLIVION

void PrintItemType(TESForm * form);
const char GetSeparatorChar(Script* script);		// '|' character can't be typed at the console, so
const char* GetSeparatorChars(Script* script);	//	  allow '@' to be substituted in commands called from console

#endif

class StringFinder_CI
{
	char* m_stringToFind;
public:
	StringFinder_CI(char* str) : m_stringToFind(str)
		{	}

	bool Accept(char* str)
	{
		if (!_stricmp(str, m_stringToFind))
			return true;
		else
			return false;
	}
};

template <class Node, class Info>
class Visitor
{
	const Node* m_pHead;

	template <class Op>
	UInt32 FreeNodes(Node* node, Op &compareOp) const
	{
		static UInt32 nodeCount = 0;
		static UInt32 numFreed = 0;
		static Node* lastNode = NULL;
		static bool bRemovedNext = false;
		
		UInt32 returnCount;

		if (node->Next())
		{
			nodeCount++;
			FreeNodes(node->Next(), compareOp);
			nodeCount--;
		}

		if (compareOp.Accept(node->Info()))
		{
			if (nodeCount)
				node->Delete();
			else
				node->DeleteHead(lastNode);
			numFreed++;
			bRemovedNext = true;
		}
		else
		{
			if (bRemovedNext)
				node->SetNext(lastNode);
			bRemovedNext = false;
			lastNode = node;
		}

		returnCount = numFreed;

		if (!nodeCount)	//reset vars after recursing back to head
		{
			numFreed = 0;
			lastNode = NULL;
			bRemovedNext = false;
		}

		return returnCount;
	}

	class AcceptAll {
	public:
		bool Accept(Info* info) {
			return true;
		}
	};

	class AcceptEqual {
		const Info	* m_toMatch;
	public:
		AcceptEqual(const Info* info) : m_toMatch(info) { }
		bool Accept(const Info* info) {
			return info == m_toMatch;
		}
	};

	class AcceptStriCmp {
		const char * m_toMatch;
	public:
		AcceptStriCmp(const char* info) : m_toMatch(info) { }
		bool Accept(const char* info) {
			return _stricmp(info, m_toMatch) ? false : true;
		}
	};
public:
	Visitor(const Node* pHead) : m_pHead(pHead) { }
	
	UInt32 Count() const {
		UInt32 count = 0;
		const Node* pCur = m_pHead;
		while (pCur && pCur->Info() != NULL) {
			++count;
			pCur = pCur->Next();
		}
		return count;
	}

	Info* GetNthInfo(UInt32 n) const {
		UInt32 count = 0;
		const Node* pCur = m_pHead;
		while (pCur && count < n && pCur->Info() != NULL) {
			++count;
			pCur = pCur->Next();
		}
		return (count == n && pCur) ? pCur->Info() : NULL;
	}

	template <class Op>
	void Visit(Op& op) const {
		const Node* pCur = m_pHead;
		bool bContinue = true;
		while (pCur && pCur->Info() && bContinue) {
			bContinue = op.Accept(pCur->Info());
			if (bContinue) {
				pCur = pCur->Next();
			}
		}
	}

	template <class Op>
	const Node* Find(Op& op, const Node* prev = NULL) const 
	{
		const Node* pCur;
		if (!prev)
			pCur = m_pHead;
		else
			pCur = prev->next;
		bool bFound = false;
		while (pCur && !bFound)
		{
			if (!pCur->Info())
				pCur = pCur->Next();
			else
			{
				bFound = op.Accept(pCur->Info());
				if (!bFound)
					pCur = pCur->Next();
			}
		}

		return pCur;
	}

	const Node* FindString(char* str, const Node* prev = NULL) const
	{
		return Find(StringFinder_CI(str), prev);
	}

	template <class Op>
	UInt32 CountIf(Op& op) const
	{
		UInt32 count = 0;
		const Node* pCur = m_pHead;
		while (pCur)
		{
			if (pCur->Info() && op.Accept(pCur->Info()))
				count++;
			pCur = pCur->Next();
		}
		return count;
	}

	const Node* GetLastNode() const
	{
		const Node* pCur = m_pHead;
		while (pCur && pCur->Next())
			pCur = pCur->Next();
		return pCur;
	}

	void RemoveAll() const
	{
		FreeNodes(const_cast<Node*>(m_pHead), AcceptAll());
	}

	template <class Op>
	UInt32 RemoveIf(Op& op)
	{
		return FreeNodes(const_cast<Node*>(m_pHead), op);
	}

	bool Remove(const Info* toRemove)
	{
		return RemoveIf(AcceptEqual(toRemove)) ? true : false;
	}

	bool RemoveString(const char* toRemove)
	{
		return RemoveIf(AcceptStriCmp(toRemove)) ? true : false;
	}

	void Append(Node* newNode)
	{
		Node* lastNode = const_cast<Node*>(GetLastNode());
		if (lastNode == m_pHead)
			lastNode->DeleteHead(newNode);
		else
			lastNode->SetNext(newNode);
	}

	template <class Op>
	UInt32 GetIndexOf(Op& op)
	{
		UInt32 idx = 0;
		const Node* pCur = m_pHead;
		while (pCur && pCur->Info() && !op.Accept(pCur->Info()))
		{
			idx++;
			pCur = pCur->Next();
		}

		if (pCur && pCur->Info())
			return idx;
		else
			return -1;
	}
};

std::string GetOblivionDirectory(void);

const char * GetDXDescription(UInt32 keycode);

namespace MersenneTwister
{

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s);

/* initialize by an array with array-length */
void init_by_array(unsigned long init_key[], int key_length);

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void);

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void);

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void);

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void);

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void);

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void);

};

#if OBLIVION
#if _DEBUG
#define DEBUG_PRINT(x, ...) { Console_Print((x), __VA_ARGS__); _MESSAGE((x), __VA_ARGS__); }
#else
#define DEBUG_PRINT(x, ...) { }
#endif	//_DEBUG
#else
#define DEBUG_PRINT(x, ...) { }
#endif	//OBLIVION

#define SIZEOF_ARRAY(arrayName, elementType) (sizeof(arrayName) / sizeof(elementType))

// Intellisense likes to crash VS when I try to use Oblivion_DynamicCast so this just saves me some frustration
#define OBLIVION_CAST(obj, from, to) (to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

// alternative to strtok; doesn't modify src string, supports forward/backward iteration
class Tokenizer
{
public:
	Tokenizer(const char* src, const char* delims);
	~Tokenizer();

	// these return the offset of token in src, or -1 if no token
	UInt32 NextToken(std::string& outStr);
	UInt32 PrevToken(std::string& outStr);

private:
	void MoveToFirstToken();

	std::string m_delims;
	size_t		m_offset;
	std::string m_data;
};

bool ci_equal(char ch1, char ch2);

// Generic error/warning output
// provides a common way to output errors and warnings
class ErrOutput
{
	typedef void (* _ShowError)(const char* msg);
	typedef bool (* _ShowWarning)(const char* msg);		// returns true if user requests to disable warning

	_ShowError		ShowError;
	_ShowWarning	ShowWarning;
public:
	ErrOutput(_ShowError errorFunc, _ShowWarning warningFunc);

	struct Message
	{
		const char*		fmt;
		bool			bCanDisable;
		bool			bDisabled;
	};

	void Show(Message& msg, ...);
	void Show(const char* msg, ...);
	void vShow(Message& msg, va_list args);
	void vShow(const char* msg, va_list args);
};

// this has been tested to work for non-varargs functions
// varargs functions end up with 'this' passed as the last parameter (ie. probably broken)
// do NOT use with classes that have multiple inheritance

// if many member functions are to be declared, use MEMBER_FN_PREFIX to create a type with a known name
// so it doesn't need to be restated throughout the member list

// all of the weirdness with the _GetType function is because you can't declare a static const pointer
// inside the class definition. inlining automatically makes the function call go away since it's a const

#define MEMBER_FN_PREFIX(className)	\
	typedef className _MEMBER_FN_BASE_TYPE

#define DEFINE_MEMBER_FN_LONG(className, functionName, retnType, address, ...)		\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);			\
																					\
	inline _##functionName##_type * _##functionName##_GetPtr(void)					\
	{																				\
		static const UInt32 _address = address;										\
		return (_##functionName##_type *)&_address;									\
	}

#define DEFINE_MEMBER_FN(functionName, retnType, address, ...)	\
	DEFINE_MEMBER_FN_LONG(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, __VA_ARGS__)

#define CALL_MEMBER_FN(obj, fn)	\
	((*(obj)).*(*((obj)->_##fn##_GetPtr())))

// ConsolePrint() limited to 512 chars; use this to print longer strings to console
void Console_Print_Long(const std::string& str);

// thread-safe template versions of ThisStdCall()

__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(); } u = { _f };
    return ((T*)_t->*u.m)();
}

template <typename T1>
__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t,T1 a1)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1); } u = { _f };
    return ((T*)_t->*u.m)(a1);
}

template <typename T1,typename T2>
__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t,T1 a1,T2 a2)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1,T2); } u = { _f };
    return ((T*)_t->*u.m)(a1,a2);
}

template <typename T1,typename T2,typename T3>
__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t,T1 a1,T2 a2,T3 a3)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1,T2,T3); } u = { _f };
    return ((T*)_t->*u.m)(a1,a2,a3);
}

template <typename T1,typename T2,typename T3,typename T4,typename T5>
__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t,T1 a1,T2 a2,T3 a3,T4 a4,T5 a5)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1,T2,T3,T4,T5); } u = { _f };
    return ((T*)_t->*u.m)(a1,a2,a3,a4,a5);
}

template <typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
__forceinline UInt32 ThisStdCall(UInt32 _f,void* _t,T1 a1,T2 a2,T3 a3,T4 a4,T5 a5, T6 a6)
{
    class T {}; union { UInt32 x; UInt32 (T::*m)(T1,T2,T3,T4,T5,T6); } u = { _f };
    return ((T*)_t->*u.m)(a1,a2,a3,a4,a5,a6);
}