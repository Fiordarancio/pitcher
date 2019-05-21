//----------------------------------------------------------------------------
// HELLO_CHAR
// Test on 3 simple letters: I, L, T
// We build a training set with different types of these letters (possibly
// with some noise too) and we give them to a neural net to eat them
//----------------------------------------------------------------------------
#include "../pnetlib.h"

#define DIM_CHAR		8
#define DIM_LABEL		3
#define NUM_SAMPLES		10

#define BATCHES			10
#define MAX_EPOCHS		20
#define LEARNING_RATE	0.001
#define MOMENTUM		0.001
#define MIN_ERR			0

char*	weight_file = "logs/hello_testchar__wg.txt";	// weights & biases 
char*	prologue_file = "logs/hello_testchar__pr.txt";	// net structure
char*	lerr_file = "logs/hello_testchar_locerr.txt";	// local error over batches
char*	gerr_file = "logs/hello_testchar_glberr.txt";	// global error over epoch

// takes square matrix and give baseline
void store_I (float* matrix, int dim)
{
	int		i, j;
	float 	choice;
	
	choice = ((float)rand()) / ((unsigned int) RAND_MAX);
	if (choice > 0.5)
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (j == dim/2)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim + j] = 0.0;
			}
		}
	}
	else
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (j == dim/2 || j == (dim/2-1))
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim + j] = 0.0;
			}
		}
	
	}
}

// takes square matrix, dimension, noise as probability to get a 0
void store_noisy_I (float* matrix, int dim, float noise_percentage)
{
	int		i, j;
	float	choice;
	for (i=0; i<dim; i++)	
	{
		for (j=0; j<dim; j++)
		{
			if (j>dim/3 && j<(2*dim)/3)
			{
				choice = ((float)rand()) / ((unsigned int) RAND_MAX);
				if (choice > noise_percentage)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim +j] = 0.0;
			}
			else
				matrix[i*dim +j] = 0.0;
		}
			
	}
}

void store_L (float* matrix, int dim)
{
	int		i, j;
	float 	choice;
	
	choice = ((float)rand()) / ((unsigned int) RAND_MAX);
	if (choice > 0.5)
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (j==0 || i==dim-1)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim + j] = 0.0;
			}
		}
	}
	else
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (j<(dim/4) || i>(3*dim)/4)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim + j] = 0.0;
			}
		}
	
	}
}

// takes square matrix, dimension, noise as probability to get a 0
void store_noisy_L (float* matrix, int dim, float noise_percentage)
{
	int		i, j;
	float	choice;
	for (i=0; i<dim; i++)	
	{
		for (j=0; j<dim; j++)
		{
			if (j<=dim/3 || i>=dim/3)
			{
				choice = ((float)rand()) / ((unsigned int) RAND_MAX);
				if (choice > noise_percentage)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim +j] = 0.0;
			}
			else
				matrix[i*dim +j] = 0.0;
		}
			
	}
}

void store_T (float* matrix, int dim)
{
	int		i, j;
	float 	choice;
	
	choice = ((float)rand()) / ((unsigned int) RAND_MAX);
	if (choice > 0.5)
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (i==0)
					matrix[i*dim+j] = 1.0;
				else
				{
					if (j == dim/2)
						matrix[i*dim + j] = 1.0;
					else 
						matrix[i*dim + j] = 0.0;
				}
			}
		}
	}
	else // bigger
	{
		for (i=0; i<dim; i++)
		{
			for (j=0; j<dim; j++)
			{
				if (i<2)
					matrix[i*dim + j] = 1.0;
				else
				{
					if (j == dim/2 || j == (dim/2-1))
						matrix[i*dim + j] = 1.0;
					else 
						matrix[i*dim + j] = 0.0;					
				}
			}
		}	
	}
}

// takes square matrix, dimension, noise as probability to get a 0
void store_noisy_T (float* matrix, int dim, float noise_percentage)
{
	int		i, j;
	float	choice;
	for (i=0; i<dim; i++)	
	{
		for (j=0; j<dim; j++)
		{
			choice = ((float)rand()) / ((unsigned int) RAND_MAX);
			if (i<=dim/3)
			{
				if (choice > noise_percentage)
					matrix[i*dim + j] = 1.0;
				else 
					matrix[i*dim + j] = 0.0;
			}
			else
			{
				if (j>dim/3 &&  j<(2*dim)/3)
				{
					if (choice > noise_percentage)
						matrix[i*dim + j] = 1.0;
					else 
						matrix[i*dim +j] = 0.0;
				}
				else
					matrix[i*dim +j] = 0.0;
			}
		}
			
	}
}




void print_char (float* matrix, int r, int c, char* letter)
{
	int i, j;
	printf("Printing %s\n", letter);
	for (i=0; i<r; i++)
	{
		for (j=0; j<c; j++)
			printf("%.0f ", matrix[i*c + j]);
		printf("\n");
	}
	printf("\n");
}

int main()
{
	struct example* training_set = NULL;
	int 			train_size = 0;
	int 			j, k;
	float			matrix [DIM_CHAR*DIM_CHAR];
	float			label [DIM_LABEL];
	p_net*			network;
	int				nhd, bt, max_ep;
	float			lr, mm, min_err;
	int 			err;
	float 			failure_rate;
	
	printf("Building training set of letters...\n");
	for (k=0; k<NUM_SAMPLES; k++)
	{
		store_I (matrix, DIM_CHAR);
		for (j=1; j<DIM_LABEL; j++)
			label[j] = 0.0;
		label[0] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "I");*/
		
		store_noisy_I (matrix, DIM_CHAR, 0.1);
		for (j=1; j<DIM_LABEL; j++)
			label[j] = 0.0;
		label[0] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "I noisy");*/
				
		// store a base L and base T
		store_L (matrix, DIM_CHAR);
		for (j=0; j<DIM_LABEL; j++)
			label[j] = 0.0;
		label[1] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "L");*/

		store_noisy_L (matrix, DIM_CHAR, 0.1);
		for (j=0; j<DIM_LABEL; j++)
			label[j] = 0.0;
		label[1] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "L noisy");*/
		
		store_T (matrix, DIM_CHAR);
		for (j=0; j<DIM_LABEL-1; j++)
			label[j] = 0.0;
		label[2] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "T");*/

		store_noisy_T (matrix, DIM_CHAR, 0.1);
		for (j=0; j<DIM_LABEL-1; j++)
			label[j] = 0.0;
		label[2] = 1.0; 
		training_set = insert_example (training_set, matrix, DIM_CHAR*DIM_CHAR, label, DIM_LABEL);
		train_size++;
/*		print_char (matrix, DIM_CHAR, DIM_CHAR, "T noisy");*/
	}
	printf("We got a training set of %d valid couples\n", train_size);
	
	// create network
	network = p_net_create();
	p_net_init(network, 2);

	printf("Insert number of neurons of hidden layer: ");
	if (scanf("%d", &nhd) <= 0)
		nhd = 1;
		
	add_layer (network, 0, nhd, DIM_CHAR*DIM_CHAR, sigmoid, ddx_sigmoid);
	add_layer (network, 1, DIM_LABEL, nhd, sigmoid, ddx_sigmoid);	
	print_netinfo(network);
	
	// train
	printf("Insert number of batches: ");
	if (scanf("%d", &bt) <= 0) 
		bt = BATCHES;
	printf("Insert max epoch to reach while training: ");
	if (scanf("%d", &max_ep) <= 0) 
		max_ep = MAX_EPOCHS;
	printf("Insert learning rate: ");
	if (scanf("%f", &lr) <= 0)
		lr = LEARNING_RATE;
	printf("Insert momentum: ");
	if (scanf("%f", &mm) <= 0)
		mm = MOMENTUM;	
	printf("Insert min error for early stopping: ")	;
	if (scanf("%f", &min_err) <= 0)
		min_err = MIN_ERR;
	printf("\nApplying learning rate %f and momentum %f\n", lr, mm);
	printf("With %d batches, we have %d samples per batch (surplus: %d)\n", bt, train_size/bt, (train_size%bt));
	printf("We stop at epoch %d or under error %f\n", max_ep, min_err);
	
	// training algorithm (backprobagation for updating weights)
	printf("Saving error over %s and %s...\n", lerr_file, gerr_file);
	p_net_train_SGD (network, max_ep, bt, lr, mm, min_err, &training_set, train_size, lerr_file, gerr_file);
		
	// save results
	printf("Saving weights over %s and %s...\n\n", weight_file, prologue_file);
	if ((err = save_network (network, weight_file, prologue_file)) < 0)
		fprintf(stderr, "FATAL ERROR in saving weights: %s\n", strerror(err));
	
	// check the prediction for all the training set! 
	failure_rate = 0;
	for (struct example* list=training_set; list!=NULL; list=list->next)
	{
		predict(network, list->samples, list->ns);
/*		printf("Binary prediction\n");*/
/*		get_float_binary_prediction(network, label, DIM_LABEL, 0.05);*/
/*		print_winner_pitch_verbose (label, DIM_LABEL);*/
		printf("Full float prediction values\n");
		print_last_prediction(network);
		printf("Winner-takes-all prediction\n");
		get_winner_prediction (network, label, DIM_LABEL);
		printf("[");
		for (j=0; j<DIM_LABEL; j++)
			printf(" %.1f ", label[j]);
		printf("]\nCORRECT ANSWER\n[");
		for (j=0; j<DIM_LABEL; j++)
			printf(" %.1f ", list->label[j]);
		printf("]\n");
		if (compare_labels(label, list->label, DIM_LABEL) < 0)
			failure_rate++;
	}
	printf("Failure rate over training set: %.1f%%\n", ((float)failure_rate)*100/train_size);
	
	printf("End of games\n");
	delete_all_examples(training_set);
	p_net_destroy(network);
}








