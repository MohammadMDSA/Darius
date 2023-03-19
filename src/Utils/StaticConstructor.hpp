//////////////////////////////////////////////////////////////////////////////
// AUTHOR:      Hugo González Castro
// TITLE:       Static Constructor in C++
// DESCRIPTION: An easy way to implement static constructors and
//              static destructors in standard C++.
// VERSION:     v1.2 - 2011/11/27
// LICENSE:     CPOL (Code Project Open License).
//              Please, do not remove nor modify this header.
// URL:         http://www.codeproject.com/KB/cpp/StaticConstructor.aspx
//////////////////////////////////////////////////////////////////////////////
#ifndef _STATIC_CONSTRUCTOR_H_
#define _STATIC_CONSTRUCTOR_H_



//////////////////////////////////////////////////////////////////////////////
// DECLARATIONS (macros to use):
//////////////////////////////////////////////////////////////////////////////


// REQUIRED macro to invoke the static constructor/destructor of a class:
// place the call to this macro out of the definition of the class, in a .CPP file
// (avoid using it in .H files unless they are included only once).
// Make sure you put this AFTER the initialization of the static data members!
// For templates, should be called with alias (typedef) for the template instaces.
//////////////////////////////////////////////////////////////////////////////
#define INVOKE_STATIC_CONSTRUCTOR(ClassName) \
		INVOKE_STATIC_CONSTRUCTOR_EXPANDED(ClassName)

#define STATIC_CONST_DEST() \
STATIC_CONSTRUCTOR(); \
STATIC_DESTRUCTOR()


// OPTIONAL macros to help to declare the header of the static constructor/destructor:
// place the calls to these macros inside the definition of the class/template, in a .H file
//////////////////////////////////////////////////////////////////////////////
#define STATIC_CONSTRUCTOR()  static void StaticConstructor()
#define STATIC_DESTRUCTOR()   static void StaticDestructor()


// OPTIONAL macro to declare static Data-Function members with inline initialization:
// place the call to this macro inside the definition of the class/template.
//////////////////////////////////////////////////////////////////////////////
// STATIC DF MEMBER (static Data-Function member):
// "a static function member with a static local (data) variable declared inside".
// The TypeName can be or not a CONST type.
//////////////////////////////////////////////////////////////////////////////
#define STATIC_DF_MEMBER(TypeName, DFMemberName, InitValue) \
		STATIC_DF_MEMBER_EXPANDED(TypeName, DFMemberName, InitValue)


// OPTIONAL macros to run code at startup and finishup. Place outside classes.
// Place the static code inside {} after one of these macros.
//////////////////////////////////////////////////////////////////////////////
#define STATIC_STARTUP_CODE()   STATIC_STARTUP_CODE_EXPANDED()
#define STATIC_FINISHUP_CODE()  STATIC_FINISHUP_CODE_EXPANDED()



//////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION (do not directly use these macros):
//////////////////////////////////////////////////////////////////////////////


// Definition of special class to invoke
// the static constructor/destructor from
// its own non-static constructor/destructor.
template<typename ClassName>
class StaticInvoker
{
public:
	// Default Constructor:
	StaticInvoker()
	{
		// Call to the static constructor of ClassName:
		ClassName::StaticConstructor();
	}

	// Destructor:
	virtual ~StaticInvoker()
	{
		// Call to the static destructor of ClassName:
		ClassName::StaticDestructor();
	}
};


// Macro with expanded name:
#define INVOKE_STATIC_CONSTRUCTOR_EXPANDED(ClassName) \
		/* Single instance of this invoker class, so its constructor is called only once */ \
		StaticInvoker<ClassName> staticInvokerOf_##ClassName;


// STATIC DF MEMBER (static Data-Function member):
// "a static function member with a static local (data) variable declared inside".
// The TypeName can be or not a CONST type.
#define STATIC_DF_MEMBER_EXPANDED(TypeName, DFMemberName, InitValue) \
		static TypeName& DFMemberName() \
		{ \
			static TypeName DFMemberName(InitValue); \
			return DFMemberName; \
		}


// Expanded macro to run code at startup.
#define STATIC_STARTUP_CODE_EXPANDED() \
		class StartUpCode \
		{ \
		public: \
			StartUpCode(); \
		}; \
		StartUpCode myStartUpCode; \
		StartUpCode::StartUpCode()


// Expanded macro to run code at finishup.
#define STATIC_FINISHUP_CODE_EXPANDED() \
		class FinishUpCode \
		{ \
		public: \
			FinishUpCode() {}; \
			virtual ~FinishUpCode(); \
		}; \
		FinishUpCode myFinishUpCode; \
		FinishUpCode::~FinishUpCode()



#endif // _STATIC_CONSTRUCTOR_H_
