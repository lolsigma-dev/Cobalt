#include "VEH.hpp"

bool handled = false;

auto Alert(PEXCEPTION_POINTERS exception_pointers) -> long __stdcall {
		CRITICAL_SECTION section;
		std::stringstream message("");

		if (!handled)
			InitializeCriticalSection(&section);

		EnterCriticalSection(&section);
		if (handled)
			LeaveCriticalSection(&section);

		handled = true;

		// exception
		message << "An unexpected error has occured!" << std::endl << std::endl;
		message << "Exception code: " << exception_pointers->ExceptionRecord->ExceptionCode << std::endl;
		message << "Exception address: " << exception_pointers->ExceptionRecord->ExceptionAddress << std::endl << std::endl;

		// R<letters>
		message << "RAX: " << exception_pointers->ContextRecord->Rax << std::endl;
		message << "RBX: " << exception_pointers->ContextRecord->Rbx << std::endl;
		message << "RCX: " << exception_pointers->ContextRecord->Rcx << std::endl;
		message << "RDX: " << exception_pointers->ContextRecord->Rdx << std::endl;
		message << "RDI: " << exception_pointers->ContextRecord->Rdi << std::endl;
		message << "RSI: " << exception_pointers->ContextRecord->Rsi << std::endl;
		message << "RBP: " << exception_pointers->ContextRecord->Rbp << std::endl;
		message << "RSP: " << exception_pointers->ContextRecord->Rsp << std::endl << std::endl;

		// R<numbers>
		message << "R8: " << exception_pointers->ContextRecord->R8 << std::endl;
		message << "R9: " << exception_pointers->ContextRecord->R9 << std::endl;
		message << "R10: " << exception_pointers->ContextRecord->R10 << std::endl;
		message << "R11: " << exception_pointers->ContextRecord->R11 << std::endl;
		message << "R12: " << exception_pointers->ContextRecord->R12 << std::endl;
		message << "R13: " << exception_pointers->ContextRecord->R13 << std::endl;
		message << "R14: " << exception_pointers->ContextRecord->R14 << std::endl;
		message << "R15: " << exception_pointers->ContextRecord->R15 << std::endl;

		message << std::endl << "Please report this to the developer" << std::endl;

		MessageBox(0, message.str().c_str(), "Cobalt Crash", MB_ICONERROR | MB_OK);

		return EXCEPTION_CONTINUE_EXECUTION;
}

auto CVEH::Start() -> bool {
		SetUnhandledExceptionFilter(Alert);
		return true;
}
