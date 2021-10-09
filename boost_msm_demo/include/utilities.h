#ifndef UTILITIES_H
#define UTILITIES_H

/************* COUT in custome colors *****************************************/
/* How to use:                                                                */
/*  std::cout<< CLEAR;                                                        */
/*  std::cout<< COUT_CUS_RED << "printing in red"                             */
/******************************************************************************/
#define D_COUT_CUS_RESET       " \033[0m "
#define D_COUT_CUS_BLACK       " \033[30m "         /* Black */
#define D_COUT_CUS_RED         " \033[31m "         /* Red */
#define D_COUT_CUS_GREEN       " \033[32m "         /* Green */
#define D_COUT_CUS_YELLOW      " \033[33m "         /* Yellow */
#define D_COUT_CUS_BLUE        " \033[34m "         /* Blue */
#define D_COUT_CUS_MAGENTA     " \033[35m "         /* Magenta */
#define D_COUT_CUS_CYAN        " \033[36m "         /* Cyan */
#define D_COUT_CUS_WHITE       " \033[37m "         /* White */
#define D_COUT_CUS_BOLDBLACK   " \033[1m\033[30m "  /* Bold Black */
#define D_COUT_CUS_BOLDRED     " \033[1m\033[31m "  /* Bold Red */
#define D_COUT_CUS_BOLDGREEN   " \033[1m\033[32m "  /* Bold Green */
#define D_COUT_CUS_BOLDYELLOW  " \033[1m\033[33m "  /* Bold Yellow */
#define D_COUT_CUS_BOLDBLUE    " \033[1m\033[34m "  /* Bold Blue */
#define D_COUT_CUS_BOLDMAGENTA " \033[1m\033[35m "  /* Bold Magenta */
#define D_COUT_CUS_BOLDCYAN    " \033[1m\033[36m "  /* Bold Cyan */
#define D_COUT_CUS_BOLDWHITE   " \033[1m\033[37m "  /* Bold White */
#define D_COUT_CUS_CLEAR       " \033[2J "          /* clear screen escape code */

#define TEMI_LOG(lev)     std::cout
#define TEMI_BOOL(lev, b) std::cout
#endif //UTILITIES_H
