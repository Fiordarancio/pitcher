#include "pnet.h"

//------------------------------------------
// COMMON OBJECTIVE FUNCTIONS
// (should add softmax)
//------------------------------------------
float relu(float x)
{
	return ((x > 0)? x:0);
}
float ddx_relu(float x)
{
	return ((x > 0)? 1:0);
}

float signum(float x)
{
	return (x >= 0)? 1.0 : (-1.0);
}
float ddx_signum(float x) 
{ 
	return (x = 0); // pay attention
} 

float sigmoid(float x)
{
	return (1.0/(1.0 + exp(-x)));
}
float ddx_sigmoid(float x)
{
	return (sigmoid(x)*(1.0 - sigmoid(x)));
}

float tangh(float x)
{
	return tanh(x);
}
float ddx_tangh(float x)
{
	return (1.0/pow(cosh(x),2));
}

// alternative: efficient but less safe
objective functions[] = { relu, signum, sigmoid, tangh, ddx_relu, ddx_signum, ddx_sigmoid, ddx_tangh };

/*float (*select_function(func_id id)) (float) */
objective select_function(func_id id)
{
	switch (id)
	{
		case RELU:
			return relu;
		case SIGNUM:
			return signum;
		case SIGMOID:
			return sigmoid;	
		case TANH:
			return tangh;		
		case DDX_RELU:
			return ddx_relu;
		case DDX_SIGNUM:
			return ddx_signum;
		case DDX_SIGMOID:
			return ddx_sigmoid;
		case DDX_TANH:
			return ddx_tangh;
		default:
			return NULL;
	}
	return NULL;
}

objective get_function(int index)
{
	assert (index>=0 && index<NFUNC);
	return functions[index];
}

func_id get_func_id (objective func)
{
	if (func == relu)
		return RELU;
	if (func == signum)
		return SIGNUM;
	if (func == sigmoid)
		return SIGMOID;
	if (func == tangh)
		return TANH;
	if (func == ddx_relu)
		return DDX_RELU;
	if (func == ddx_signum)
		return DDX_SIGNUM;
	if (func == ddx_sigmoid)
		return DDX_SIGMOID;
	if (func == ddx_tangh)
		return DDX_TANH;
	return FNONDEF;
}

//---------------------------------------------------------------------------------------------------------------
// BASE CONSTRUCTORS AND 'DESTRUCTORS'
//---------------------------------------------------------------------------------------------------------------
perceptron* perceptron_create()
{
	return (perceptron*) malloc (sizeof(perceptron));
}

void perceptron_destroy(perceptron* p)
{
	free(p->weights);
	// free(p); // should be done outside
}

void perceptron_init(perceptron* p, int n, float (*function)(float), float (*derivative)(float))
{
	assert(n>0); 	// don't return NULL, abort
	int i; 
	
	p->nweights = n;
	p->bias = ((float) rand()) / ((unsigned int) RAND_MAX+1); 	// fixed trainable input, with -1 when predicting
	p->a = 0;													// nothing valuable
	p->y = 0; 
	p->d = 0;

	p->weights = (float*) malloc(sizeof(float)*p->nweights);
	for (i=0; i<n; i++)
	{
	// give to weights random values uniformly distributed in [0,1]
		p->weights[i] = ((float) rand()) / ((unsigned int) RAND_MAX+1);
/*		p->weights[i] = (float) rand() / ((unsigned int) RAND_MAX + 1);		*/
/*		p->weights[i] /= pow(2,32) - 1; // that's nonsense '*/
/*		printf("w[%d] %f\n", i, p->weights[i]);*/
/*		p->weights[i] = 0.0;*/
	}
	p->fx 	= function;
	p->dfx 	= derivative;
}

void perceptron_load(perceptron* p, float* w, float b)
{
	int i;
	p->bias = b;
	for (i=0; i<p->nweights; i++)
		p->weights[i] = w[i]; // risk of segfault if dim is not correct
}

p_layer* p_layer_create()
{
	return (p_layer*) malloc (sizeof(p_layer));
}

void p_layer_destroy(p_layer* layer)
{
	int i;
	for (i=0; i<layer->nperc; i++)
		free(layer->perceptrons[i].weights);
	free(layer->perceptrons);
	// layer must be freed from outside
}

// init a layer of n perceptrons, whose weights are m
void p_layer_init(	p_layer* layer, int n_perc, int weights_per_perc, float (*common_f)(float), float (*common_df)(float))
{
	assert(n_perc > 0);
	assert(weights_per_perc > 0);
	
	int i;	
	layer->nperc = n_perc;
	layer->perceptrons = (perceptron*) malloc (sizeof(perceptron)*n_perc);
	for (i=0; i<n_perc; i++)
		perceptron_init(&layer->perceptrons[i], weights_per_perc, common_f, common_df);
}
 
// create a netork: after a p_net has been dynamically instantiated, layers are added 
// sequentially. We arrange this stuff in a 'new'-like way
p_net* p_net_create()
{
	p_net* net = (p_net*) malloc (sizeof(p_net));
	net->nlayers = 0;
	net->layers = NULL;
	return net;
}

void p_net_init (p_net* net, int n_layers)
{
	net->nlayers = n_layers;
	net->layers = (p_layer*) malloc (sizeof(p_layer) * n_layers);
}

void p_net_destroy(p_net* net)
{
	int l, j;
	for (l=0; l<net->nlayers; l++)
	{
		for (j=0; j<net->layers[l].nperc; j++)
			free(net->layers[l].perceptrons[j].weights);
		free(net->layers[l].perceptrons);
	}
	free(net->layers);
	free(net);
}

void add_layer (p_net* net, int index, int n_perc, int weights_per_perc, objective f, objective df)
{
	assert (index >= 0 && index < net->nlayers);
	p_layer_init(&net->layers[index], n_perc, weights_per_perc, f, df);
}

// deprecated
/*void add_layer (p_layer* layer, p_net* net, int index)*/
/*{*/
/*	net->layers = realloc (net->layers, sizeof(p_layer) * (net->nlayers + 1));*/
/*	net->layers[net->nlayers] = *layer;*/
/*	net->nlayers++;*/
/* 	assert (index >= 0 && index < net->nlayers);*/
/* 	net->layers[index] = *layer;*/
/*}*/

// evaluate output of perceptron given N sorted inputs
void evaluate(perceptron* p, float* x, int n)
{
	assert(n==p->nweights);

	p->a = -p->bias;
	while (n--) 
		p->a += x[n]*p->weights[n];
	p->y = p->fx(p->a);
/*	printf("p->a: %f p->y: %f\n", p->a, p->y);*/
}

// evaluate output of a layer that is not the first one
void evaluate_perc(perceptron *p, perceptron *perceptrons, int num_perceptrons)
{
	assert(num_perceptrons==p->nweights);
	int j;

	p->a = -p->bias;
	for (j=0; j<num_perceptrons; j++)
		p->a += perceptrons[j].y*p->weights[j];
	p->y = p->fx(p->a);
}

// output for the whole network
void predict(p_net* net, float* x, int n)
{
	int 	l, j;	
	
	// input layer by first
	for (j=0; j<net->layers[0].nperc; j++)
	{
/*		printf("Pred %d of input layer\n", j);*/
		evaluate(&net->layers[0].perceptrons[j], x, n);
	}
	// all the others
	for (l=1; l<net->nlayers; l++)
		for (j=0; j<net->layers[l].nperc; j++)
			evaluate_perc(&net->layers[l].perceptrons[j], net->layers[l-1].perceptrons, net->layers[l-1].nperc);
}

// puts into an array of integers approximating the output
void get_binary_prediction (p_net* net, int* prediction, int dim_prediction, float thresh)
{
	int			j;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
/*	int*		prediction = (int*) malloc (sizeof(int) * top_layer->nperc);*/
	assert		(dim_prediction == top_layer->nperc);
	
	for (j=0; j<top_layer->nperc; j++)
	{
		if (top_layer->perceptrons[j].y >= thresh)
			prediction[j] = 1;
		else
			prediction[j] = 0;
	}
}

// the threshold we use for getting the prediction can vary!
void get_float_binary_prediction (p_net* net, float* prediction, int dim_prediction, float thresh)
{
	float 		val;
	int			j;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
	assert (dim_prediction == top_layer->nperc);
	
	for (j=0; j<top_layer->nperc; j++)
	{
		val = top_layer->perceptrons[j].y;
		if (val >= thresh)
			prediction[j] = 1.0;
		else
			prediction[j] = 0.0;
	}
}

void get_winner_prediction (p_net* net, float* prediction, int dim_prediction)
{
	float 		max = -1;
	float 		val;
	int 		j;
	int			winner_index = -1;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
	assert (dim_prediction == top_layer->nperc);
	
	for (j=0; j<top_layer->nperc; j++)
	{
		prediction[j] = 0.0; // init
		val = top_layer->perceptrons[j].y;
		if (val > max)
		{
			max = val;
			winner_index = j;
		}
		else 
			if (val == max) // the very first can't be
				winner_index = -1;
	}
	if (winner_index != -1)
		prediction[winner_index] = 1.0;
		
}

void print_last_prediction (p_net* net)
{
	int 		j;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
	
	printf("[");
	for (j=0; j<top_layer->nperc; j++)
		printf(" %f ", top_layer->perceptrons[j].y);
	printf("]\n");
}

//---------------------------------------------------------------------------------
// UPDATE WEIGHTS USING BACKPROPAGATION
//---------------------------------------------------------------------------------
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

// UPDATE: we now apply crossentropy
//-----------------------------------------------------------------------------------

// minibatch propagation for averaging
float*** backpropagation_delta (p_net* net, float* target, int dim_target, float* inputs, int dim_inputs, float lr)
{
	// target vector MUST have the same dimension as the number of PERCEPTRONS of output layer
	assert(dim_target == net->layers[net->nlayers-1].nperc);
	// input vector MUST have the same dimension as the number of WEIGHTS of any perceptron in the input layer
	assert(dim_inputs == net->layers[0].perceptrons[0].nweights);
	
	int 	l, j, i;
	
	// each time we call backpropagation_delta(), we are going to return a "matrix" of #layers rows, 
	// each row has #neurons elements in which the relative DeltaWji are stored
	float*** DeltaWji = (float***) malloc (sizeof(float**) * net->nlayers);
	
	// Special case: only one layer
	if (net->nlayers == 1)
	{
		p_layer* top_layer = &net->layers[0];
		for (j=0; j<top_layer->nperc; j++)
		{
			perceptron* pj = &top_layer->perceptrons[j];
			pj->d = (target[j] - pj->y) * pj->dfx(pj->a);
/*			printf("      [output] pj->d: %.4f pj->y: %.4f tj: %.1f pj->a: %.4f pj->dfx(a): %.4f\n", pj->d, pj->y, target[j], pj->a, pj->dfx(pj->a));*/
		}	
	} // the next for loop exits immediately being l==0
	
	// Starting by the top layer, we consider a pair at each step and compute dj: layer l and layer l-1 
	for (l=net->nlayers-1; l>0; l--)
	{
		p_layer* currlayer = &net->layers[l];		// current layer
		p_layer* prevlayer = &net->layers[l-1];		// previous layer
		
		// if we are on the top layer, we need dj
		if (l==net->nlayers-1)
		{
			for (j=0; j<currlayer->nperc; j++)
			{
				perceptron* pj = &currlayer->perceptrons[j];
				pj->d = (target[j] - pj->y) * pj->dfx(pj->a);
/*				printf("      [output] pj->d: %.4f pj->y: %.4f tj: %.1f pj->a: %.4f pj->dfx(a): %.4f\n", pj->d, pj->y, target[j], pj->a, pj->dfx(pj->a));*/
			}

		}
		// in all cases evaluate di of your previous layer
		for (i=0; i<prevlayer->nperc; i++)
		{
			perceptron* pi = &prevlayer->perceptrons[i];
			float sum = 0;
			for (j=0; j<currlayer->nperc; j++)
				sum += currlayer->perceptrons[j].weights[i] * currlayer->perceptrons[j].d;
			pi->d = pi->dfx(pi->a) * sum;
/*			printf("      [eval for layer %d] pi->d: %.4f pi->a: %.4f pi->dfx(a): %.4f sum: %.4f\n", l-1, pi->d, pi->a, pi->dfx(pi->a), sum);*/

		}
		
		// once all di are stored, we memorize the delta rule in DeltaWji
		DeltaWji[l] = (float**) malloc (sizeof(float*) * currlayer->nperc);
		for (j=0; j<currlayer->nperc; j++)
		{
			perceptron* p = &currlayer->perceptrons[j];
			DeltaWji[l][j] = (float*) malloc (sizeof(float) * p->nweights);
			for (i=0; i<p->nweights; i++) 
				DeltaWji[l][j][i] = lr * p->d * prevlayer->perceptrons[i].y;
		}
	}
	
	// the bottom layer (the input one) has not been modified we do it now as last
	p_layer* inputlayer = &net->layers[0];
	DeltaWji[0] = (float**) malloc (sizeof(float*) * inputlayer->nperc);
	for (j=0; j<inputlayer->nperc; j++)
	{
		DeltaWji[0][j] = (float*) malloc (sizeof(float) * inputlayer->perceptrons[j].nweights);
		for (i=0; i<inputlayer->perceptrons[j].nweights; i++)
			DeltaWji[0][j][i] = lr * inputlayer->perceptrons[j].d * inputs[i];
	}
	
	return DeltaWji;
}

// once all p->d have been evaluated using backpropagation_delta, we can give also the
// modification to be applied on the bias
float** backpropagation_bias (p_net* net, float lr)
{
	int 		j, l;
	float**		DeltaBias = (float**) malloc (sizeof(float**) * net->nlayers);

	for (l=0; l<net->nlayers; l++)
	{
		DeltaBias[l] = (float*) malloc (sizeof(float) * net->layers[l].nperc);
		for (j=0; j<net->layers[l].nperc; j++)
		{
			perceptron* p = &net->layers[l].perceptrons[j];
			DeltaBias[l][j] = -lr * p->d;
		}
	}
	return DeltaBias;
}

// compute error: it is the average squared error on example target
float quadratic_error (p_net* net, float* target, int dim)
{
	assert(dim == net->layers[net->nlayers-1].nperc);
	
	int			j;
	float 		sum = 0;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
	for (j=0; j<top_layer->nperc; j++)
		sum += pow( (target[j] - top_layer->perceptrons[j].y), 2);
	sum /= top_layer->nperc; 
	return sum;
}

// average crossentropy on output neurons
float crossentropy_error (p_net* net, float* target, int dim)
{
	assert(dim == net->layers[net->nlayers-1].nperc);
	
	int			j;
	float 		sum = 0;
	p_layer*	top_layer = &net->layers[net->nlayers-1];
	for (j=0; j<top_layer->nperc; j++)
	{
		perceptron* p = &top_layer->perceptrons[j];
		sum += -target[j] * log(p->y) - (1 - target[j]) * log(1 - p->y);
	}
	sum /= top_layer->nperc;
	return sum;
}


// open a file and save weights of a single perceptron
// We use high-level API trhough streams
int save_weights(perceptron *p, char* fname)
{
	int err;
	FILE *fd = fopen(fname, "w+"); // overwrite
	assert(fd!=NULL);
	
	for (int i=0; i<p->nweights; i++)
	{
		if ((err = fprintf(fd, "%f\n", p->weights[i])) < 0)
		{	
			int errvalue = errno;
			fprintf(stderr, "Error while printing weights: %s", strerror(errvalue));
			return err;
		}
	}
	fclose(fd);
	return NN_SUCCESS;
}

// save weights of a whole network: pass names of file for weights and for prologue
int save_network (p_net* net, char* wgfile, char* prfile) 
{
	int err, errval;
	int l, j, i;

	FILE *fd = fopen(prfile, "w+");
	assert(fd!=NULL);

	// By first, we write down a prologue. The output is something like
	// Layers:<numlayers>/n<numperceptrons>:<numweightsperperceptron>,<enum_fx>,<enum_dfx>
	// Hence an example with our layers could be
	// Layers:3
	// 1024:2205,0,3	-> 2205 percs, function RELU, dfx DDX_RELU
	// 2048:1024,0,3
	// 12:2048,2,5		-> 12 percs, function SIGMOID, dfx DDX_SIGMOID
	if ((err = fprintf(fd, "Layers:%d\n", net->nlayers)) < 0)	
	{
		fprintf(stderr, "Error while printing total number of layers\n");
		fclose(fd);
		return err;
	}
	for (l=0; l<net->nlayers; l++)
	{
		p_layer* layer = &net->layers[l];
		perceptron* p = &layer->perceptrons[0];
		if ((err = fprintf(	fd, "%d:%d,%d,%d\n", layer->nperc, p->nweights, get_func_id(p->fx), get_func_id(p->dfx))) < 0)
		{
			fprintf(stderr, "Error while printing information about a perceptron of layer %d: %s\n", l, strerror(err));
			fclose(fd);
			return err;
		}
	}
	// close prologue and open weight file
	fclose (fd);
	fd = fopen(wgfile, "w+");
	assert (fd!=NULL);
	// we save weights from top to bottom in this CSV format: row is the layer, ';' separates perceptrons, ',' separates weights
	// while the very first value before the list of weights is about the bias of the perceptron under consideration
	// Example:
	// [layer L(row)]perc[0]bias,perc[0]weight[0], ..perc[0]weight[I]; ...perc[j]bias,perc[J]weight[0], ...perc[J]weight[I]
	// ...[layer 0]p[0]w[0],p[0]w[1], ..p[0]w[I];p[1]w[0], ..p[1]w[I]; ..p[J]w[0], ..p[J]w[I]
	for (l=0; l<net->nlayers; l++)
	{	
		p_layer* layer = &net->layers[l];
		for (j=0; j<layer->nperc; j++)
		{
			perceptron* p = &layer->perceptrons[j];
			if ((err = fprintf(fd, "%f,", p->bias)) < 0)
			{
				errval = errno;
				fprintf(stderr, "Error while printing bias of perceptron %d in layer %d: %s\n", j, l, strerror(errval));
				fclose(fd);
				return err;
			}
			for(i=0; i<p->nweights; i++)
			{
				if (i==p->nweights-1)
				{
					if ((err = fprintf(fd, "%f;", p->weights[i])) < 0)
					{
						errval = errno;
						fprintf(stderr, "Error while printing weight %d of perceptron %d of layer %d: %s\n", i, j, l, strerror(errval));
						fclose(fd);
						return err;
					}
				}
				else
				{
					if ((err = fprintf(fd, "%f,", p->weights[i])) < 0)
					{
						errval = errno;
						fprintf(stderr, 
							"Error while printing weight %d of perceptron %d of layer %d: %s\n", i, j, l, strerror(errval));
						fclose(fd);
						return err;
					}
				}
				
			}
		}
		if ((err = fprintf(fd, "\n")) < 0)
		{
			errval = errno;
			fprintf(stderr, "Error while changing from layer %d: %s\n", l, strerror(errval));
			fclose(fd);
			return err;
		}
	}
	fclose(fd);
	return NN_SUCCESS;
} 

//------------------------------------------------------------------------
// CREATE A NEW NETWORK WITH DATA LOADED FROM FILE
//------------------------------------------------------------------------
p_net* load_network (char* wgfile, char* prfile) 
{
	p_net*		net = NULL;
	
	FILE* 		fd;
	int 		nlayers;
	int			nperceptrons; 	// per layer
	int 		nweights;		// per perceptron in layer
	int 		fid = FNONDEF;	// function ids to be passed to get the pointer
	int			dfid = FNONDEF;
	int			l, j, i;		// iterators
	int			readcount;		
	char		info[8];		// used for get rid of separators
	float		value;			// save float value read for weight

	// By first, prepare to read prologue file	
	fd = fopen(prfile, "r+");
	assert(fd!=NULL);
	
	readcount = fscanf(fd, "%7s", info);
	if (readcount <= 0)
	{
		fprintf(stderr, "No bytes returned (readcount: %d)\n", readcount);
		fclose(fd);
	}
	else
	{
		net = p_net_create();
		if (strcmp(info, "Layers:") == 0)
		{
			assert(fscanf(fd, "%d", &nlayers) == 1);
			p_net_init(net, nlayers);
			for (l=0; l<nlayers; l++)
			{
				assert(fscanf(fd, "%d:%d,%d,%d", &nperceptrons, &nweights, &fid, &dfid) == 4);
				add_layer(net, l, nperceptrons, nweights, select_function((func_id)fid), select_function((func_id)dfid));
			}
		}
		else 
		{
			fprintf(stderr, "Error: wrong format (%s)\n", info);
			fclose(fd);
			p_net_destroy(net);
			return NULL;
		}
	}
	
	// Second step: open and load all specific weights
	fclose(fd);
	fd = fopen(wgfile, "r+");
	assert(fd!=NULL);
	
	// for each layer (separated by '\n')
	//  for each perceptron (separated by ';')
	//	 for each weight (separated by ',') 	> save the correspondent weight
	for (l=0; l<net->nlayers; l++)
	{
		p_layer* layer = &net->layers[l];
		for (j=0; j<layer->nperc; j++)
		{
			perceptron* p = &layer->perceptrons[j];
			if ((readcount = fscanf(fd, "%f,", &value)) < 0) // take already a ,
			{
				fprintf(stderr, "Error while reading bias of perc %d in layer %d\n", j, l);
				fclose(fd);
				p_net_destroy(net);
				return NULL;
			}
			p->bias = value;
			for (i=0; i<p->nweights; i++)
			{
				if ((readcount = fscanf(fd, "%f", &value)) < 0)
				{
					fprintf(stderr, "Error while reading weight %d of perc %d in layer %d\n", i, j, l);
					fclose(fd);
					p_net_destroy(net);
					return NULL;
				}
				p->weights[i] = value;
				readcount = fscanf(fd, "%c", &info[0]); // remove separator
				assert(info[0] == ';' || info[0] == ',');
				if ((info[0] == ';') && (i!=p->nweights-1))
				{
					fprintf(stderr, "Wrong format (found %c) while i is %d (weights %d)\n", info[0], i, p->nweights-1);
					fclose(fd);
					p_net_destroy(net);
					return NULL;
				}
			}
		}
	}
	fclose(fd);
	return net;
}

//---------------------------------------------
// PRINT NETWORK CHARACTERISTICS
//---------------------------------------------
void print_netinfo (p_net* net) 
{
	int l;
	
	printf("\nMULTILAYER PERCEPTRON NETWORK\n");
	printf("Total layers: %d\n", net->nlayers);
	for (l=0; l<net->nlayers; l++)
		printf("    [Layer %d] Perceptrons: %d (weights per neuron %d)\n", l, net->layers[l].nperc, net->layers[l].perceptrons[0].nweights);
}

void print_netinfo_verbose (p_net* net)
{
	int l, j, i;
	
	print_netinfo(net);
	
	printf("FULL NETWORK MAP\n");
	for (l=0; l<net->nlayers; l++)
	{
		printf("LAYER %d\n", l);
		p_layer* layer = &net->layers[l];
		for (j=0; j<layer->nperc; j++)
		{
			printf("  PERCEPTRON %d\n", j);
			perceptron* p = &layer->perceptrons[j];
			printf("    bias: %f\n", p->bias);
			for (i=0; i<p->nweights; i++)
			{
				printf("    w%d: %f", i, p->weights[i]);
				if (i%5 == 4)
					printf("\n");
			}
			printf("\n");
		}
	}
}
 
