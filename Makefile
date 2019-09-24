PROG	=	dual-dhclient
SRCS	=	main.c
NO_MAN	?=	yes
BINDIR	?=	/usr/local/sbin

.include <bsd.prog.mk>
