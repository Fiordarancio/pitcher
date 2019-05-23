//-------------------------------------------------------------------------------------
// Perceptron NETwork LIBrary
// 
// File for managing training and predictions on a perceptron-based network as it 
// has been defined into p_net.h. The training is generalized over examples that are
// examples of a float array of samples and a float array of labels.
// The user has responsability on the size of samples and labels (usually intended
// as categorical ones)
//-------------------------------------------------------------------------------------
#ifndef PNETLIB_H
#define PNETLIB_H

#include "pnet.h"

struct example 
{
	float* 	samples;	// samples are values (even only 1) constituting the example
	int		ns;			// dimension of the array
	int		nl;
	float* 	label;	 	// array constituting a label. Generally, the i-th value of
						// label represents the i-th category 
	
	struct example*	next;
};

// insert example on the top of a list
//void			insert_example (struct example** list, float* smp, int ns, float* lab, int nl);
struct example* insert_example (struct example* list, float* smp, int ns, float* lab, int nl);
// from a list, get i-th example
struct example* get_example (struct example* list, int index);
// delete and free list
void 			delete_all_examples (struct example* list);
// naive prints for example
void 			print_example_label (struct example* ex);
void 			print_examples (struct example* head, int verbose);
// shuffle list
struct example*	shuffle_examples (struct example* head, int dim);
int				compare_labels (float* lab1, float* lab2, int dim);

//-------------------------------------------------------------------------------------

// normalize training set
void 	normalize_example (struct example* ex);
void 	normalize_examples (struct example* head, int list_size);

// standardize training set
void	standardize_example (struct example* ex);
void 	standardize_examples (struct example* head, int list_size);
// print error on a given file
// X: epoch or batch; Y: modulus of the error vector
int 	print_exerr (FILE *fd, int x, float error);
//------------------------------------------------------------------------------------------------------------------------------
// TRAINING
//   -> Stochastic Gradient Descent, Minibatch algorithm
//	 -> Batch error and epoch error is saved on files for future plotting
//------------------------------------------------------------------------------------------------------------------------------
void 	p_net_train_SGD (	p_net* net, int epochs, int batches, float lrate, float mom, float min_err,
							struct example** tset, int tsize, char* lf, char* gf);
float	p_net_train_on_example (p_net* net, struct example* ex, float*** Dw, float** Db, float lrate/*, float mom*/);

#endif

