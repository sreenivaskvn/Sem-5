#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	printf(1, "Starting the parent process\n");
	int ret, i, prio;
	for(i = 0; i < 2; i++){
		ret = fork();
		if(ret == 0)
			break;
	}
	if(ret != 0)
		return 0;

	printf(1,"Child %d starting ...", i);
	if(i == 0){
		prio = setprio(1000);
	}
	prio = getprio();
	for(double j = 0; j < 1000000000; j++);

	printf(1,"Child %d with priority %d ending ...", i, prio);
}