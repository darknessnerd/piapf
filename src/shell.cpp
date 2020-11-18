#include "shell.h"

#include <iostream>
#include <unistd.h>


using namespace std;
using namespace shell;

cmd::cmd(string command, bool throws) : command_(command)
{
    char line[500];
    if ( !(pipe_ = (FILE*)popen(command.c_str(),"r")) )
        throw cmdException("Couldn't open pipe.",(long)pipe_);

    while( fgets(line, sizeof line, pipe_))
        output_.append(line);

    int status = pclose(pipe_);
    exit_code_ = WEXITSTATUS(status);
    if(exit_code_ != 0 && throws)
        throw cmdException(command_, output_, exit_code_);
}

cmd::~cmd()
{
    // Closing is allready done in the constructor
    // pclose(pipe_);
}


string cmd::str()
{
    return output_;
}

bool cmd::ok()
{
    return exit_code_ == 0;
}

ostream& shell::operator<<(ostream& str,cmd sh)
{
    return str << sh.output_;
};


/** CMD EXCEPTION */
cmdException::cmdException(string command, string output,int code)
    : runtime_error{"Command '"+command+"' returned error: "
                    +to_string(code)+" "+output},
      exit_code_(code)
{}

cmdException::cmdException(string what,int code)
    : runtime_error{what},exit_code_(code)
{}


int cmdException::exit_code()
{
    return exit_code_;
}
