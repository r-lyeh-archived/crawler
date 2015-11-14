#include <stddef.h>
#include "tinyfiledialogs.h"
int main()
{
  tinyfd_forceConsole = tinyfd_messageBox("Hello World",
    "force dialogs into console mode?\
    \n\t(it's better if dialog is installed)",
    "yesno", "question", 0);
  char const * lThePassword =  tinyfd_inputBox(
    "a password box","your password will be revealed",NULL);
  if ( lThePassword )
    tinyfd_messageBox("your password is", lThePassword, "ok", "info", 1);
}