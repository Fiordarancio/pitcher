#include "mutils.h"

// Euler forward
float euler_trapezoid_short (short* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs((float)values[i]) + fabs((float)values[i+1])) * h)/2;
	return sum;
}

float euler_trapezoid_int (int* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs((float)values[i]) + fabs((float)values[i+1])) * h)/2;
	return sum;
}

float euler_trapezoid_float (float* values, float h, int n)
{
	int 	i;
	float 	sum = 0;
	for (i=0; i<(n-1); i++)
		sum += ((fabs(values[i]) + fabs(values[i+1])) * h)/2;
	return sum;
}

// evaluate mean of values
float mean (float* values, int n)
{
	float mean = 0;
	for (int i=0; i<n; i++)
		mean += values[i];
	return (mean / n);
}

float mean_abs (int* values, int n)
{
	float mean = 0;
	for (int i=0; i<n; i++)
		mean += abs(values[i]);
	return (mean / n);

}

// shuffle the array (found on the web) This is a very straightforward EXAMPLE we have to modify
void shuffle(int *array, size_t n)
{
    if (n > 1) 
    {
        int i;
        for (i=0; i < n-1; i++) 
        {
          int j = i + rand() / (RAND_MAX / (n-i) + 1);
/*		  int j = i + rand() * (n-i) / (RAND_MAX +1); // Tommi suggests */
			// written like this RAND_MAX goes on overflow
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

// shuffle a list
struct elem* shuffle_list (struct elem* head, int dim)
{
	// special cases
	if (head == NULL || head->next == NULL || dim < 2)
		return head;
	struct elem* list;
	if (dim == 2)
	{
		list = head->next;
		list->next = head;
		head->next = NULL;
		head = list;
		return head;
	}
	
	struct elem* 	previ = NULL;
	struct elem* 	prevj = NULL;
	struct elem* 	elemi = NULL;
	struct elem* 	elemj = NULL;
	int 			i,j,k;
	for (i=0; i < dim-1; i++) 
    {
    	// select indexes
		k = i + rand() / (RAND_MAX / (dim-i) + 1);
		j = i + (int) ( (float)rand() * (float)(dim-i) / (RAND_MAX-1) ); // my update
		printf("[%d] Old bugged evaluation was %d. Update after Tommi reviewed: %d. ", i, k, j);
		j = i + rand() * (dim-i) / ((unsigned int)RAND_MAX + 1);
		printf("Tommi suggestion: %d\n", j);
		if (j > dim-1)
		{
			printf("Invalid new index\n");
			return head;
		}
		if (i!=j)
		{
			// identify the elements we need
			list = head;
		  	for (k=0; k<dim; k++)
		  	{
		  		if (k==i-1)
		  			previ = list;
		  		if (k==i) 
		  			elemi = list;
		  		if (k==j-1)
		  			prevj = list;
		  		if (k==j)
		  			elemj = list;
		  		list = list->next;
		  	}
		  	// test if i is not zero
		  	if (previ==NULL)
		  		head = elemj;
		  	else
				previ->next = elemj;
			// exchange the others
			prevj->next = elemi;
			struct elem* app = elemi->next;
			elemi->next = elemj->next;
			elemj->next = app;

			printf("[");
			for (struct elem* e=head; e!=NULL; e=e->next)
				printf("%d ", e->info);
			printf("]\n");
		}
    }
	return head;
}


