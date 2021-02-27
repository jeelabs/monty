namespace arch {
    auto cliTask () -> monty::Stacklet*;

    void init (int =0);
    void idle ();
    void done [[noreturn]] ();
}
