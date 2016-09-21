#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	dummy();
	printf(1, "KVN's first user program on xv6\n");
	int prio = setprio(10);
	prio = getprio();
	printf(1, "Priority of the process is : ");
	printf(1, "%d\n", prio);
	exit();
}