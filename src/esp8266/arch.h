enum class RunMode { Run, GC, Idle, Sleep, Done, };
extern void archMode (RunMode); // show status on LEDs, if available

extern Monty::Module const m_machine;
