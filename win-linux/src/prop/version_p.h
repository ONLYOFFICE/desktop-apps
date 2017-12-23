#ifndef VERSION_PRIVATE_H
#define VERSION_PRIVATE_H

#ifdef __NCT
# undef VER_COMPANYNAME_STR
# undef VER_LEGALCOPYRIGHT_STR
# undef VER_COMPANYDOMAIN_STR
# undef ABOUT_COPYRIGHT_STR

# define VER_COMPANYNAME_STR        "Novie kommunikacionnie tehnologii JSC\0"
# define VER_LEGALCOPYRIGHT_STR     "Novie kommunikacionnie tehnologii JSC, 2017\0"
# define VER_COMPANYDOMAIN_STR      "www.onlyoffice.ru\0"
# define ABOUT_COPYRIGHT_STR        VER_LEGALCOPYRIGHT_STR
#endif

#endif // VERSION_PRIVATE_H
