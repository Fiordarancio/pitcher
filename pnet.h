//-------------------------------------------------------------------------------------
// Perceptron NETwork 
// 
// File for building up and manage the NN based on perceptrons. 
//-------------------------------------------------------------------------------------
#ifndef PNET_H
#define PNET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <float.h>

#include "debug.h"

#define NN_SUCCESS 	0
#define NN_FAILURE 	(-1)
#define OPEN		0
#define CLOSE		1
#define PRINT		2
#define	NFUNC		8

//------------------------------------------
// COMMON OBJECTIVE FUNCTIONS
// (should add softmax)
//------------------------------------------
// select a function given an ID
typedef enum { 
	FNONDEF = -1,
	RELU, SIGNUM, SIGMOID, TANH, 
	DDX_RELU, DDX_SIGNUM, DDX_SIGMOID, DDX_TANH
} func_id;

typedef float (*objective) (float); // let things be confortable

// given an id or a function pointer, say which function or id it reflects
/*float (*select_function(func_id id)) (float) */
objective select_function(func_id id);
objective get_function(int index);
func_id get_func_id (objective func);

float relu(float x);
float ddx_relu(float x);

float signum(float x);
float ddx_signum(float x);

float sigmoid(float x);
float ddx_sigmoid(float x);

float tangh(float x);
float ddx_tangh(float x);

//------------------------------------------------------------
// PERCEPTRON
// By now, every neuron in our network is a perceptron.
// However, we know that labels are binary (0/1)
//------------------------------------------------------------
typedef struct {
	int 	nweights;
	float* 	weights; 			// weights are dynamically allocated
	float 	bias; 				// this is fixed
	float 	a;					// sum of all weights*inputs + bias
	float	y;					// fx(a)
	float	d;					// delta in backpropagation
	float	(*fx) (float x);	// pointer to the objective function
	float 	(*dfx)(float x);	// derivative of fx (for backpropagation)
} perceptron;

//--------------------------------------------------------------------------------------------------
// LAYER OF PERCEPTRONS AND MULTI-LAYER PERCEPTRON NETWORK
// List the layers and dynamically decide for each how many perceptrons we use, in how many layers
//--------------------------------------------------------------------------------------------------
typedef struct {
	int 		nperc;
	perceptron* perceptrons; // array of perceptrons
	
} p_layer; // layer of perceptrons

typedef struct {
	int 		nlayers;		// number of layers
	p_layer* 	layers;			// list of perceptron per layer
} p_net; // perceptron-based network

//---------------------------------------------------------------------------------------------------------------
// BASE CONSTRUCTORS AND 'DESTRUCTORS'
//---------------------------------------------------------------------------------------------------------------
perceptron*	perceptron_create();
void 		perceptron_destroy(perceptron* p);
void 		perceptron_init(perceptron* p, int n, float (*function)(float), float (*derivative)(float));
void		perceptron_load(perceptron* p, float* w, float b);

p_layer* 	p_layer_create();
void 		p_layer_destroy(p_layer* layer);
void 		p_layer_init(	p_layer* layer, int n_perc, int weights_per_perc, float (*common_f)(float), float (*common_df)(float));
 
p_net* 		p_net_create();
void 		p_net_destroy(p_net* net);
// creates array of layers but initialization must be done outside
void 		p_net_init (p_net* net, int n_layers);
// adds a new layer at given index, providing all information needed by p_layer_init
void 		add_layer (p_net* net, int index, int n_perc, int weights_per_perc, objective f, objective df);
//void 		add_layer (p_layer* layer, p_net* net, int index);


//---------------------------------------------------------------------------------------------------------------
// OPERATING WITH THE NETWORK
//---------------------------------------------------------------------------------------------------------------
// evaluate output of perceptron given N sorted inputs
void 	evaluate(perceptron* p, float* x, int n);
void 	evaluate_perc(perceptron *p, perceptron *perceptrons, int num_perceptrons);
// output for the whole network
void 	predict(p_net* net, float* x, int n);
void	get_binary_prediction(p_net* net, int* prediction, int dim_prediction, float thresh);
void	get_float_binary_prediction(p_net* net, float* prediction, int dim_prediction, float thresh);
void 	get_winner_prediction (p_net* net, float* prediction, int dim_prediction);

//----------------------------------------------------------------------------------------------------
// We apply backpropagation to evaluate the delta rule among the hidden
// perceptrons. Hence, this is the last function to be called, the one
// which updated weight according to values that have been computed in
// the previous step.
// 	- net is the network to train
// 	- target and dim_target refer the array of labels
//  - inputs and dim_inputs refer to input chunks
// 	- lr is the learning rate

// we use the common notation by which j is the index of the jth neuron into
// current layer and i is the ith weight, hence coming from ith neuron of
// the previous layer. Hence
// 	- dj is the backpropagation delta of pj, which is the jth perceptron 
//	  of layer under consideration
//  - xi is the input coming from perceptron i of the previous layer: in
//	  other words, it is input to weight i of current perceptron

// The first version of the function evaluates backpropagation and applies it
// immediately. The second one instead returns the whole "matrix" DeltaWji
//----------------------------------------------------------------------------------------------------
void 		backpropagation (p_net* net, float* target, int dim_target, float* inputs, int dim_inputs, float lr);
float***	backpropagation_delta (p_net* net, float* target, int dim_target, float* inputs, int dim_inputs, float lr);
float** 	backpropagation_bias (p_net* net, float lr);
// evaluate error for example given
float 		quadratic_error (p_net* net, float* target, int dim);
float 		crossentropy_error (p_net* net, float* target, int dim);
//--------------------------------------------------------------------------------------------------
// PRINT WEIGHTS AND LOAD WEIGHTS FROM FILE
//--------------------------------------------------------------------------------------------------
// open a file and save weights of a single perceptron. We use high-level API trhough streams
int 		save_weights (perceptron *p, char* fname);
// save weights of a whole network onto a prologue file and a weight file
// we save weights from top to bottom in this CSV format: row is the layer, ';' separates perceptrons, ',' separates weights
// [layer L(row)]perceptron[0]weight[0], ..perceptron[0]weight[I]; ...perceptron[J]weight[0], ...perceptron[J]weight[I]
// ...[layer 0]p[0]w[0],p[0]w[1], ..p[0]w[I];p[1]w[0], ..p[1]w[I]; ..p[J]w[0], ..p[J]w[I]
int 		save_network (p_net* net, char* wgfile, char* prfile);
p_net*		load_network (char* wgfile, char* prfile);
void 		print_netinfo (p_net* net); 
void 		print_netinfo_verbose (p_net* net);
void 		print_last_prediction (p_net* net);
 
#endif

