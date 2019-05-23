//---------------------------------------------------------------------------------------
// TESTING pnet for OR
// In this test, we build up 2 simple networks: a sigle-perceptron network and a 
// 2by2 network like in hello_testand. We check if our network can learn an OR
// Since the bias is trained, we don't register it anymore
//---------------------------------------------------------------------------------------
#include "../pnet.h"
#include "../pnetlib.h"

#include <stdlib.h>
#include <stdio.h>

#define DIM_SMP 	2
#define DIM_LAB2	2
#define DIM_LAB1	1	

int main()
{
	p_net*			net1, *net2;
	struct example*	training_set1 = NULL;
	struct example* training_set2 = NULL;
	struct example* test_set1 = NULL;
	struct example* test_set2 = NULL;	
	int				train_size1, train_size2, test_size1, test_size2;
	int 			i, j;
	int				ep, bt; 	// epochs, batches, 
	float			lr, mm, me; // learning rate, momentum, min err
	char*			lfile1 = "logs/hello_testor1_locerr.txt";
	char*			gfile1 = "logs/hello_testor1_glberr.txt";
	char*			lfile2 = "logs/hello_testor2_locerr.txt";
	char*			gfile2 = "logs/hello_testor2_glberr.txt";
	float			smp [DIM_SMP];
	float			lab2 [DIM_LAB2];	// net2 is categorical
	float			lab1;				// net1 is binary
	float 			failure_rate;
	
	// prepare network with one perceptron only
	net1 = p_net_create();
	net2 = p_net_create();
	p_net_init(net1, 1);
	p_net_init(net2, 2);
	
	// first net
	add_layer (net1, 0, 1, 2, sigmoid, ddx_sigmoid);
	printf("SINGLE PERCEPTRON NET - ");
	print_netinfo(net1);
	
	// second net
	add_layer (net2, 0, 2, 2, sigmoid, ddx_sigmoid);
	add_layer (net2, 1, 2, 2, sigmoid, ddx_sigmoid);
	printf("2BY2 NET - ");
 	print_netinfo(net2);
 	
	// prepare training and test sets
	train_size1 = test_size1 = train_size2 = test_size2 = 0;
	for (i=0; i<40; i++)
	{
		smp [0]		= smp [1] = 0;
		lab2 [0] 	= 1.0;
		lab2 [1] 	= 0.0;
		lab1 		= 0;
		training_set1 = insert_example (training_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
		training_set2 = insert_example (training_set2, smp, DIM_SMP, lab2, DIM_LAB2);
		train_size1++; train_size2++;
		if (i<20)
		{
			test_set1 = insert_example (test_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
			test_set2 = insert_example (test_set2, smp, DIM_SMP, lab2, DIM_LAB2);
			test_size1++; test_size2++;
		}
	}
	for (i=0; i<80; i++)
	{
		smp [0]	 = (float) (rand() % 2);
		smp [1]	 = (float) (rand() % 2);
		if ((smp[0] || smp[1]) == 1)
		{
			lab2 [0] = 0.0;
			lab2 [1] = 1.0;
			lab1	 = 1.0;
		}
		else
		{
			lab2 [0] = 1.0;
			lab2 [1] = 0.0;
			lab1	 = 0.0;

		}
				
		training_set1 = insert_example (training_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
		training_set2 = insert_example (training_set2, smp, DIM_SMP, lab2, DIM_LAB2);
		train_size1++; train_size2++;
		if (i<40)
		{
			test_set1 = insert_example (test_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
			test_set2 = insert_example (test_set2, smp, DIM_SMP, lab2, DIM_LAB2);
			test_size1++; test_size2++;
		}
	}
	for (i=0; i<40; i++)
	{
		smp [0]  = smp [1] = 1.0;
		lab2 [0] = 0.0;
		lab2 [1] = 1.0;
		lab1	 = 1.0;
		
		training_set1 = insert_example (training_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
		training_set2 = insert_example (training_set2, smp, DIM_SMP, lab2, DIM_LAB2);
		train_size1++; train_size2++;
		if (i<20)
		{
			test_set1 = insert_example (test_set1, smp, DIM_SMP, &lab1, DIM_LAB1);
			test_set2 = insert_example (test_set2, smp, DIM_SMP, lab2, DIM_LAB2);
			test_size1++; test_size2++;
		}
	}
	printf("Training set has %d examples while test set has got %d\n", train_size1, test_size1);	
	
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
	printf("With %d batches, we have %d samples per batch (surplus: %d)\n", bt, train_size1/bt, (train_size1%bt));
	printf("We stop at epoch %d or under error %f\n", ep, me);
	
	// train
	printf("TRAINING SINGLE PERCEPTRON NETWORK...\n");
	p_net_train_SGD (net1, ep, bt, lr, mm, me, &training_set1, train_size1, lfile1, gfile1);
	printf("\nTRAINING 2BY2 NETWORK...\n");
	p_net_train_SGD (net2, ep, bt, lr, mm, me, &training_set2, train_size2, lfile2, gfile2);
	
	//--------------------------------------------------------------------------------------------------	
	// NETWORK 1 -> recycle lab1
	//--------------------------------------------------------------------------------------------------
	// check over training set
	failure_rate = 0;
	printf("\nNETWORK PREDICTION OVER TRAINING SET\n");
	for (i=0; i<train_size1; i++)
	{
		struct example* ex = get_example(training_set1, i);
		assert(ex != NULL);
		predict(net1, ex->samples, DIM_SMP);
		get_binary_prediction(net1, (int*)&lab1, DIM_LAB1, 0.5);
/*		printf("[%d] Looking at (%d, %d) [%d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0]);*/
/*		printf("network predicts: [%.1f] -> [%d] ", net1->layers[net1->nlayers-1].perceptrons[0].y, predicted[0]);*/
		int isRight = 1;
		if (lab1 != (int)ex->label[0])
			isRight = 0;
/*		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");*/
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over training set: %.1f%%\n", (failure_rate/train_size1)*100);
	failure_rate = 0;
	
	// check over test set
	printf("\nNETWORK PREDICTION OVER TEST SET\n");
	for (i=0; i<test_size1; i++)
	{
		struct example* ex = get_example(test_set1, i);
		predict(net1, ex->samples, DIM_SMP);
		get_binary_prediction(net1, (int*)&lab1, DIM_LAB1, 0.5);
/*		printf("[%d] Looking at (%d, %d) [%d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0]);*/
/*		printf("network predicts: [%.1f] -> [%d] ", net1->layers[net1->nlayers-1].perceptrons[0].y, predicted[0]);*/
		int isRight = 1;
		if (lab1 != (int)ex->label[0])
			isRight = 0;
/*		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");*/
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over test set: %.1f%%\n", (failure_rate/train_size1)*100);	

	//------------------------------------------------------------------------------------------------------------------	
	// NETWORK 2 -> recycle lab2
	//------------------------------------------------------------------------------------------------------------------
	// check over training set
	failure_rate = 0;
	printf("\nNETWORK PREDICTION OVER TRAINING SET\n");
	for (i=0; i<train_size2; i++)
	{
		struct example* ex = get_example(training_set2, i);
		assert(ex != NULL);
		predict(net2, ex->samples, DIM_SMP);
		get_binary_prediction(net2, (int*)lab2, DIM_LAB2, 0.5);
/*		printf("[%d] Looking at (%d, %d) [%d, %d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0], (int)ex->label[1]);*/
/*		printf("network predicts: [%.1f, %.1f] -> [%d, %d] ", net2->layers[net2->nlayers-1].perceptrons[0].y, net2->layers[net2->nlayers-1].perceptrons[1].y, predicted[0], predicted[1]);*/
		int isRight = 1;
		for (j=0; j<DIM_LAB2; j++)
			if (lab2[j] != (int)ex->label[j])
				isRight = 0;
/*		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");*/
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over training set: %.1f%%\n", (failure_rate/train_size2)*100);
	failure_rate = 0;
	
	// check over test set
	printf("\nNETWORK PREDICTION OVER TEST SET\n");
	for (i=0; i<test_size1; i++)
	{
		struct example* ex = get_example(test_set2, i);
		predict(net2, ex->samples, DIM_SMP);
		get_binary_prediction(net2, (int*)lab2, DIM_LAB2, 0.5);
/*		printf("[%d] Looking at (%d, %d) [%d, %d], ", i, (int)ex->samples[0], (int)ex->samples[1], (int)ex->label[0], (int)ex->label[1]);*/
/*		printf("network predicts: [%.1f, %.1f] -> [%d, %d] ", net2->layers[net2->nlayers-1].perceptrons[0].y, net2->layers[net2->nlayers-1].perceptrons[1].y, predicted[0], predicted[1]);*/
		int isRight = 1;
		for (j=0; j<DIM_LAB2; j++)
			if (lab2[j] != (int)ex->label[j])
				isRight = 0;
/*		printf("-> answer is %s\n", (isRight==0)? "WRONG :(" : "CORRECT :D");*/
		if (isRight == 0)
			failure_rate++;
	}
	printf("Failure rate over test set: %.1f%%\n", (failure_rate/train_size2)*100);	

	
	// end
	delete_all_examples(training_set1);
	delete_all_examples(training_set2);
	delete_all_examples(test_set1);
	delete_all_examples(test_set2);	
	p_net_destroy(net1);
	p_net_destroy(net2);	
	
	return 0;
}
