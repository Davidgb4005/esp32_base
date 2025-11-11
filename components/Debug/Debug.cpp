#include "Debug.hpp"
#if DEBUG
extern "C" {
    #include <errno.h>
}
#include <iostream>
#endif

void PrintReport(const char * c ,const char * error_code ,const char * c1 ,int error_code1){
    std::cout<<c<<error_code<<c1<<error_code1<<std::endl;
}
void PrintReport(const char * c ,int error_code){
    std::cout<<c<<error_code<<std::endl;
}
void PrintReport(const char * c){
    std::cout<<c<<std::endl;
}
void PrintError(const char *c ,int error_code)
{
#if DEBUG
    std::cout<<c<<std::endl;
    switch (error_code)
    {
    #ifdef EDEADLK
    case EDEADLK:
        std::cout << "Resource deadlock would occur\n";
        break;
    #endif

    #ifdef ENAMETOOLONG
    case ENAMETOOLONG:
        std::cout << "File name too long\n";
        break;
    #endif

    #ifdef ENOLCK
    case ENOLCK:
        std::cout << "No record locks available\n";
        break;
    #endif

    #ifdef ENOSYS
    case ENOSYS:
        std::cout << "Invalid system call number\n";
        break;
    #endif

    #ifdef ENOTEMPTY
    case ENOTEMPTY:
        std::cout << "Directory not empty\n";
        break;
    #endif

    #ifdef ELOOP
    case ELOOP:
        std::cout << "Too many symbolic links encountered\n";
        break;
    #endif

    #ifdef EWOULDBLOCK
    case EWOULDBLOCK:
        std::cout << "Operation would block\n";
        break;
    #endif

    #ifdef ENOMSG
    case ENOMSG:
        std::cout << "No message of desired type\n";
        break;
    #endif

    #ifdef EIDRM
    case EIDRM:
        std::cout << "Identifier removed\n";
        break;
    #endif

    #ifdef ECHRNG
    case ECHRNG:
        std::cout << "Channel number out of range\n";
        break;
    #endif

    #ifdef EL2NSYNC
    case EL2NSYNC:
        std::cout << "Level 2 not synchronized\n";
        break;
    #endif

    #ifdef EL3HLT
    case EL3HLT:
        std::cout << "Level 3 halted\n";
        break;
    #endif

    #ifdef EL3RST
    case EL3RST:
        std::cout << "Level 3 reset\n";
        break;
    #endif

    #ifdef ELNRNG
    case ELNRNG:
        std::cout << "Link number out of range\n";
        break;
    #endif

    #ifdef EUNATCH
    case EUNATCH:
        std::cout << "Protocol driver not attached\n";
        break;
    #endif

    #ifdef ENOCSI
    case ENOCSI:
        std::cout << "No CSI structure available\n";
        break;
    #endif

    #ifdef EL2HLT
    case EL2HLT:
        std::cout << "Level 2 halted\n";
        break;
    #endif

    #ifdef EBADE
    case EBADE:
        std::cout << "Invalid exchange\n";
        break;
    #endif

    #ifdef EBADR
    case EBADR:
        std::cout << "Invalid request descriptor\n";
        break;
    #endif

    #ifdef EXFULL
    case EXFULL:
        std::cout << "Exchange full\n";
        break;
    #endif

    #ifdef ENOANO
    case ENOANO:
        std::cout << "No anode\n";
        break;
    #endif

    #ifdef EBADRQC
    case EBADRQC:
        std::cout << "Invalid request code\n";
        break;
    #endif

    #ifdef EBADSLT
    case EBADSLT:
        std::cout << "Invalid slot\n";
        break;
    #endif

    #ifdef EBFONT
    case EBFONT:
        std::cout << "Bad font file format\n";
        break;
    #endif

    #ifdef ENOSTR
    case ENOSTR:
        std::cout << "Device not a stream\n";
        break;
    #endif

    #ifdef ENODATA
    case ENODATA:
        std::cout << "No data available\n";
        break;
    #endif

    #ifdef ETIME
    case ETIME:
        std::cout << "Timer expired\n";
        break;
    #endif

    #ifdef ENOSR
    case ENOSR:
        std::cout << "Out of streams resources\n";
        break;
    #endif

    #ifdef ENONET
    case ENONET:
        std::cout << "Machine is not on the network\n";
        break;
    #endif

    #ifdef ENOPKG
    case ENOPKG:
        std::cout << "Package not installed\n";
        break;
    #endif

    #ifdef EREMOTE
    case EREMOTE:
        std::cout << "Object is remote\n";
        break;
    #endif

    #ifdef ENOLINK
    case ENOLINK:
        std::cout << "Link has been severed\n";
        break;
    #endif

    #ifdef EADV
    case EADV:
        std::cout << "Advertise error\n";
        break;
    #endif

    #ifdef ESRMNT
    case ESRMNT:
        std::cout << "Srmount error\n";
        break;
    #endif

    #ifdef ECOMM
    case ECOMM:
        std::cout << "Communication error on send\n";
        break;
    #endif

    #ifdef EPROTO
    case EPROTO:
        std::cout << "Protocol error\n";
        break;
    #endif

    #ifdef EMULTIHOP
    case EMULTIHOP:
        std::cout << "Multihop attempted\n";
        break;
    #endif

    #ifdef EDOTDOT
    case EDOTDOT:
        std::cout << "RFS specific error\n";
        break;
    #endif

    #ifdef EBADMSG
    case EBADMSG:
        std::cout << "Not a data message\n";
        break;
    #endif

    #ifdef EOVERFLOW
    case EOVERFLOW:
        std::cout << "Value too large for defined data type\n";
        break;
    #endif

    #ifdef ENOTUNIQ
    case ENOTUNIQ:
        std::cout << "Name not unique on network\n";
        break;
    #endif

    #ifdef EBADFD
    case EBADFD:
        std::cout << "File descriptor in bad state\n";
        break;
    #endif

    #ifdef EREMCHG
    case EREMCHG:
        std::cout << "Remote address changed\n";
        break;
    #endif

    #ifdef ELIBACC
    case ELIBACC:
        std::cout << "Cannot access needed shared library\n";
        break;
    #endif

    #ifdef ELIBBAD
    case ELIBBAD:
        std::cout << "Accessing corrupted shared library\n";
        break;
    #endif

    #ifdef ELIBSCN
    case ELIBSCN:
        std::cout << ".lib section in a.out corrupted\n";
        break;
    #endif

    #ifdef ELIBMAX
    case ELIBMAX:
        std::cout << "Too many shared libraries\n";
        break;
    #endif

    #ifdef ELIBEXEC
    case ELIBEXEC:
        std::cout << "Cannot exec shared library directly\n";
        break;
    #endif

    #ifdef EILSEQ
    case EILSEQ:
        std::cout << "Illegal byte sequence\n";
        break;
    #endif

    #ifdef ERESTART
    case ERESTART:
        std::cout << "Interrupted system call should be restarted\n";
        break;
    #endif

    #ifdef ESTRPIPE
    case ESTRPIPE:
        std::cout << "Streams pipe error\n";
        break;
    #endif

    #ifdef EUSERS
    case EUSERS:
        std::cout << "Too many users\n";
        break;
    #endif

    #ifdef ENOTSOCK
    case ENOTSOCK:
        std::cout << "Socket operation on non-socket\n";
        break;
    #endif

    #ifdef EDESTADDRREQ
    case EDESTADDRREQ:
        std::cout << "Destination address required\n";
        break;
    #endif

    #ifdef EMSGSIZE
    case EMSGSIZE:
        std::cout << "Message too long\n";
        break;
    #endif

    #ifdef EPROTOTYPE
    case EPROTOTYPE:
        std::cout << "Protocol wrong type for socket\n";
        break;
    #endif

    #ifdef ENOPROTOOPT
    case ENOPROTOOPT:
        std::cout << "Protocol not available\n";
        break;
    #endif

    #ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
        std::cout << "Protocol not supported\n";
        break;
    #endif

    #ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
        std::cout << "Socket type not supported\n";
        break;
    #endif

    #ifdef EOPNOTSUPP
    case EOPNOTSUPP:
        std::cout << "Operation not supported on transport endpoint\n";
        break;
    #endif

    #ifdef EPFNOSUPPORT
    case EPFNOSUPPORT:
        std::cout << "Protocol family not supported\n";
        break;
    #endif

    #ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
        std::cout << "Address family not supported\n";
        break;
    #endif

    #ifdef EADDRINUSE
    case EADDRINUSE:
        std::cout << "Address already in use\n";
        break;
    #endif

    #ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
        std::cout << "Cannot assign requested address\n";
        break;
    #endif

    #ifdef ENETDOWN
    case ENETDOWN:
        std::cout << "Network is down\n";
        break;
    #endif

    #ifdef ENETUNREACH
    case ENETUNREACH:
        std::cout << "Network is unreachable\n";
        break;
    #endif

    #ifdef ENETRESET
    case ENETRESET:
        std::cout << "Network dropped connection because of reset\n";
        break;
    #endif

    #ifdef ECONNABORTED
    case ECONNABORTED:
        std::cout << "Software caused connection abort\n";
        break;
    #endif

    #ifdef ECONNRESET
    case ECONNRESET:
        std::cout << "Connection reset by peer\n";
        break;
    #endif

    #ifdef ENOBUFS
    case ENOBUFS:
        std::cout << "No buffer space available\n";
        break;
    #endif

    #ifdef EISCONN
    case EISCONN:
        std::cout << "Transport endpoint already connected\n";
        break;
    #endif

    #ifdef ENOTCONN
    case ENOTCONN:
        std::cout << "Transport endpoint not connected\n";
        break;
    #endif

    #ifdef ESHUTDOWN
    case ESHUTDOWN:
        std::cout << "Cannot send after transport endpoint shutdown\n";
        break;
    #endif

    #ifdef ETOOMANYREFS
    case ETOOMANYREFS:
        std::cout << "Too many references\n";
        break;
    #endif

    #ifdef ETIMEDOUT
    case ETIMEDOUT:
        std::cout << "Connection timed out\n";
        break;
    #endif

    #ifdef ECONNREFUSED
    case ECONNREFUSED:
        std::cout << "Connection refused\n";
        break;
    #endif

    #ifdef EHOSTDOWN
    case EHOSTDOWN:
        std::cout << "Host is down\n";
        break;
    #endif

    #ifdef EHOSTUNREACH
    case EHOSTUNREACH:
        std::cout << "No route to host\n";
        break;
    #endif

    #ifdef EALREADY
    case EALREADY:
        std::cout << "Operation already in progress\n";
        break;
    #endif

    #ifdef EINPROGRESS
    case EINPROGRESS:
        std::cout << "Operation now in progress\n";
        break;
    #endif

    #ifdef ESTALE
    case ESTALE:
        std::cout << "Stale file handle\n";
        break;
    #endif

    #ifdef EUCLEAN
    case EUCLEAN:
        std::cout << "Structure needs cleaning\n";
        break;
    #endif

    #ifdef ENOTNAM
    case ENOTNAM:
        std::cout << "Not a XENIX named type file\n";
        break;
    #endif

    #ifdef ENAVAIL
    case ENAVAIL:
        std::cout << "No XENIX semaphores available\n";
        break;
    #endif

    #ifdef EISNAM
    case EISNAM:
        std::cout << "Is a named type file\n";
        break;
    #endif

    #ifdef EREMOTEIO
    case EREMOTEIO:
        std::cout << "Remote I/O error\n";
        break;
    #endif

    #ifdef EDQUOT
    case EDQUOT:
        std::cout << "Quota exceeded\n";
        break;
    #endif

    #ifdef ENOMEDIUM
    case ENOMEDIUM:
        std::cout << "No medium found\n";
        break;
    #endif

    #ifdef EMEDIUMTYPE
    case EMEDIUMTYPE:
        std::cout << "Wrong medium type\n";
        break;
    #endif

    #ifdef ECANCELED
    case ECANCELED:
        std::cout << "Operation canceled\n";
        break;
    #endif

    #ifdef ENOKEY
    case ENOKEY:
        std::cout << "Required key not available\n";
        break;
    #endif

    #ifdef EKEYEXPIRED
    case EKEYEXPIRED:
        std::cout << "Key has expired\n";
        break;
    #endif

    #ifdef EKEYREVOKED
    case EKEYREVOKED:
        std::cout << "Key has been revoked\n";
        break;
    #endif

    #ifdef EKEYREJECTED
    case EKEYREJECTED:
        std::cout << "Key rejected by service\n";
        break;
    #endif

    #ifdef EOWNERDEAD
    case EOWNERDEAD:
        std::cout << "Owner died\n";
        break;
    #endif

    #ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE:
        std::cout << "State not recoverable\n";
        break;
    #endif

    #ifdef ERFKILL
    case ERFKILL:
        std::cout << "Operation not possible due to RF-kill\n";
        break;
    #endif

    #ifdef EHWPOISON
    case EHWPOISON:
        std::cout << "Memory page has hardware error\n";
        break;
    #endif

    default:
        std::cout << "Unknown errno: " << errno << "\n";
        break;
    }
#endif
}
