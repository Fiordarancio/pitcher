//---------------------------------------------------------------------------------------
// TESTING pnet for AND 2
// Test as testand, with only 1 perceptron
//---------------------------------------------------------------------------------------
#include "../pnet.h"
#include "../pnetlib.h"

#include <stdlib.h>
#include <stdio.h>

#define DIM_SMP 2
#define DIM_LAB	1
#define DIM_SET 40

int main()
{
	p_net*			network;
	struct example*	training_set = NULL;
	struct example* test_set = NULL;
	int				train_size, test_size;
	int 			i, j;
	int				ep, bt; 	// epochs, batches, 
	float			lr, mm, me; // learning rate, momentum, min err
	char*			lfile = "logs/hello_testand2_locerr.txt";
	char*			gfile = "logs/hello_testand2_glberr.txt";
	float			smp [DIM_SMP];
	float			lab [DIM_LAB];
	int				predicted [DIM_LAB];
	float 			failure_rate;
	
	// prepare network: fixed
	network = p_net_create();
	p_net_init (network, 1);
	
	add_layer(network, 0, 1, 2, sigmoid, ddx_sigmoid);
 	print_netinfo(network);
 	
	// prepare training and test sets
	train_size = test_size = 0;
	for (i=0; i<DIM_SET/2; i++)
	{
		smp [0] = smp [1] = 0;
		lab [0] = 0.0;
		training_set = insert_example (training_set, smp, DIM_SMP, lab, DIM_LAB);
		train_size++;
		if (i<DIM_SET/4)
		{
			test_set = insert_example (test_set, smp, DIM_SMP, lab, DIM_LAB);
			test_size++;
		}
	}
	for (i=0; i<DIM_SET; i++)
	{
		smp [0] = (float) (rand() % 2);
		smp [1] = (float) (rand() % 2);
		if ((smp[0] && smp[1]) == 1)
		{
			lab [0] = 1.0;
		}
		else
		{
			lab [0] = 0.0;	
		}
		
		training_set = insert_example (training_set, smp, DIM_SMP, lab, DIM_LAB);
		train_size++;
		if (i<DIM_SET/2)
		{
			test_set = insert_example (test_set, smp, DIM_SMP, lab, DIM_LAB);
			test_size++;
		}
	}
	for (i=0; i<DIM_SET/2; i++)
	{
		smp [0] = smp [1] = 1.0;
		lab [0] = 1.0;
		training_set = insert_example (training_set, smp, DIM_SMP, lab, DIM_LAB);
		train_size++;
		if (i<DIM_SET/4)
		{
			test_set = insert_example (test_set, smp, DIM_SMP, lab, DIM_LAB);
			test_size++;
		}
	}
	printf("Training set has %d examples while test set has got %d\n", train_size, test_size);
	
	
	
	// prepare training
	printf("Insert number of batches: ");
	if (scanf("%d", &bt) < 0) 
		bt = 2;
	printf("Insert max epoch to reach while training: ");
	if (scanf("%d", &ep) < 0) 
		ep = 2;
	printf("Insert learning rate: ");
	if (scanf("%f", &lr) < 0)
		lr = 0.001;
	printf("Insert momentum: ");
	if (scanf("%f", &mm) < 0)
		mm = 0.0;	
	printf("Insert min error for early stopping: ");
	if (scanf("%f", &me) < 0)
		me = 0.001;	
	printf("\nApplying learning rate %f and momentum %f\n", lr, mm);
	printf("With %d batches, we have %d samples per batch (surplus: %d)\n", bt, train_size/bt, (train_size%bt));
	printf("We stop at epoch %d or under error %f\n\n", ep, me);

	// train
	p_net_train_SGD (network, ep, bt, lr, mm, me, &training_set, train_size, lfile, gfile);
	
	// check over training set
	failure_rate = 0;
	printf("\nNETWORK PREDICTION OVER TRAINING SET\n");
	for (i=0; i<train_size; i++)
	{
		struct example* ex = get_example(training_set, i);
		assert(ex != NULL);
		predict(network, ex->samples, DIM_SMP);
		get_binary_prediction(network, predicted, DIM_LAB, 0.5);
		printf("[%d] Looking at (%d, %d) [%d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0]);
		printf("network predicts: [%.1f] -> [%d] ", network->layers[network->nlayers-1].perceptrons[0].y, predicted[0]);
		int isRight = 1;
		for (j=0; j<DIM_LAB; j++)
			if (predicted[j] != (int)ex->label[j])
				isRight = 0;
		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over training set: %.1f%%\n", (failure_rate/train_size)*100);
	failure_rate = 0;
	
	// check over test set
	printf("\nNETWORK PREDICTION OVER TEST SET\n");
	for (i=0; i<test_size; i++)
	{
		struct example* ex = get_example(test_set, i);
		predict(network, ex->samples, DIM_SMP);
		// use predicted as predict
		get_binary_prediction(network, predicted, DIM_LAB, 0.5);
		printf("[%d] Looking at (%d, %d) [%d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0]);
		printf("network predicts: [%.1f] -> [%d] ", network->layers[network->nlayers-1].perceptrons[0].y, predicted[0]);
		int isRight = 1;
		for (j=0; j<DIM_LAB; j++)
			if (predicted[j] != (int)ex->label[j])
				isRight = 0;
		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over test set: %.1f%%\n", (failure_rate/train_size)*100);	
	
	// end
	delete_all_examples(training_set);
	delete_all_examples(test_set);
	p_net_destroy(network);
	
	return 0;
}
