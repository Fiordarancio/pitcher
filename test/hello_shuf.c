// test of shuffling with simple elements
#include "../mutils.h"
#include <stdio.h>

int main()
{
	struct elem* mylist = NULL;
	
	for (int i=0; i<20; i++)
	{
		struct elem* elem = (struct elem*) malloc (sizeof(struct elem));
		elem->info = 20-i;
		elem->next = mylist;
		mylist = elem;
	}
	printf("[");
	for (struct elem* e=mylist; e!=NULL; e=e->next)
		printf("%d ", e->info);
	printf("]\n");
	
	mylist = shuffle_list(mylist, 20);
	
	printf("[");
	for (struct elem* e=mylist; e!=NULL; e=e->next)
		printf("%d ", e->info);
	printf("]\n");
	
	return 0;
}

