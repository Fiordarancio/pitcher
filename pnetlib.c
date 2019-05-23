#include "pnetlib.h"

//-------------------------------------------------------------------------------------------------
// LIST MANAGEMENT FUNCTIONS
//-------------------------------------------------------------------------------------------------

// insert example on the top of a list
struct example* insert_example (struct example* list, float* smp, int ns, float* lab, int nl)
{
	int i;
	
	assert (ns > 0);
	assert (nl > 0);
	struct example* ex = (struct example*) malloc (sizeof(struct example));
	ex->samples = (float*) malloc (sizeof(float) * ns);
	ex->label = (float*) malloc (sizeof(float) * nl);
	ex->ns = ns;
	ex->nl = nl;
	
	for (i=0; i<ns; i++)
		ex->samples[i] = smp[i];
	for (i=0; i<nl; i++)
		ex->label[i] = lab[i];
	
	ex->next = list;
	return ex;	
/*	ex->next = *list;*/
/*	*list = ex;*/
}

/*struct example*	insert_example (struct example* list, struct example* ex)*/
/*{*/
/*	assert(ex->next == NULL);*/
/*	ex->next = list;*/
/*	return ex;*/
/*}*/

// from a list, get i-th example
struct example* get_example (struct example* list, int index)
{
	int count = 0;
	for (struct example* ex=list; ex!=NULL; ex=ex->next)
	{
		if (count == index)
			return ex;
		count ++;
	}
	return NULL; // index not present
}

// delete and free list
void delete_all_examples (struct example* list)
{
	struct example* ex=list;
	while (ex!=NULL)
	{
		struct example* app = ex->next;
		free(ex->samples);
		free(ex->label);
		free(ex);
		ex = app;
	}
}

// naive prints for example
void print_example_label (struct example* ex)
{
	int i;
	
	printf("Label: [");
	for (i=0; i<ex->nl; i++)
		printf(" %.3f ", ex->label[i]);
	printf("]");
}

void print_examples (struct example* head, int verbose)
{
	int i, j;
	
	if (head == NULL)
	{
		printf("List is empty\n");
		return;
	}
	
	i=0;
	for (struct example* list=head; list!=NULL; list=list->next)
	{
		printf("Example %d: ", ++i);
		print_example_label(list);
		if (verbose)
		{	
			printf(" Values: (");
			for (j=0; j<list->ns; j++)
				printf(" %.3f ", list->samples[j]);
			printf(")");
		}
		printf("\n");
	}
}

// shuffle NEW
struct example* shuffle_examples (struct example* head, int dim)
{
	// special cases
	if (head == NULL || head->next == NULL || dim < 2)
		return head;
		
	struct example* list;
	int 			i,j,k;
	if (dim == 2)
	{
		list = head->next;
		list->next = head;
		head->next = NULL;
		return list;
	}
	
	// verify dimension
	i=0;
	for (list=head; list!=NULL; list=list->next)
		i++;
	assert (i==dim);
	
	// do as many shuffles as the set dimension
/*	for (i=0; i<dim; i++)*/
/*	{*/
/*		list = head;*/
/*		j = rand() * dim / ((unsigned int) RAND_MAX + 1);*/
/*		// the first to reach is min between i and j*/
/*		if (j<i)*/
/*			k = j;*/
/*		else */
/*			k = i; */
/*		// append to a new list the values */
/*			*/
/*	}*/
	
	struct example* previ = NULL;
	struct example* prevj = NULL;
	struct example* elemi = NULL;
	struct example* elemj = NULL;
	for (i=0; i < dim-1; i++) 
    {
    	// select indexes: uniform randomness can end up in having i=j
    	// but in this case simply nothing needs to be done
		j = i + rand() * (dim-i) / ((unsigned int)RAND_MAX + 1);
		assert (j<dim);
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
		  	// test if previ is not null
		  	if (previ == NULL)
		  		head = elemj;
		  	else
				previ->next = elemj;
			// exchange the others
			prevj->next = elemi;
			struct example* app = elemi->next;
			elemi->next = elemj->next;
			elemj->next = app;
		}
    }
	return head;
}

// say success if labels are equal
int compare_labels (float* lab1, float* lab2, int dim)
{
	int i;
	for (i=0; i<dim; i++)
		if (lab1[i] != lab2[i])
			return -1;
	return 0;
}

//-------------------------------------------------------------------------------------
// TRAINING FUNCTIONS

//-------------------------------------------------------------------------------------
// NORMALIZE TRAINING SET (RESCALE [0,1] VALUES)
//-------------------------------------------------------------------------------------
void normalize_example (struct example* ex)
{
	float 	min, max;
	int 	i;
	// search max and min
	min = ex->samples[0];
	max = ex->samples[0];
	for (i=0; i<ex->ns; i++)
	{
		if (ex->samples[i] < min)
			min = ex->samples[i];
		if (ex->samples[i] > max)
			max = ex->samples[i];
	}
	// apply formula for all samples
	for (i=0; i<ex->ns; i++)
		ex->samples[i] = (ex->samples[i] - min) / (max - min);
}

void normalize_examples (struct example* head, int list_size)
{
	int i = 0;
	
	for (struct example* list=head; list!=NULL; list=list->next)
	{
		assert (i<list_size);
		normalize_example (get_example(head, i));
		i++;
	}
}
//-------------------------------------------------------------------------------------
// STANDARDIZE TRAINING SET (APPLY MEAN AND STD DEVIATION)
//-------------------------------------------------------------------------------------
void standardize_example (struct example* ex)
{
	int 	i;
	float 	m = 0, s = 0; // mean and std deviation
	
	for (i=0; i<ex->ns; i++)
		m += ex->samples[i];
	m /= ex->ns;
	for (i=0; i<ex->ns; i++)
		s += pow( (ex->samples[i] - m), 2);
	s /= ex->ns;
	for (i=0; i<ex->ns; i++)
		ex->samples[i] = (ex->samples[i] - m) / s;
}

void standardize_examples (struct example* head, int list_size)
{
	assert (head!=NULL);
	int			i, k;
	const int	SAMP_SIZE = head->ns; 
	float		m[SAMP_SIZE]; // mean and
	float		s[SAMP_SIZE]; // standard deviation
	for (i=0; i<SAMP_SIZE; i++) // initialization
	{
			m[i] = 0;
			s[i] = 0;
	}
	
	struct example* list;
	for (i=0; i<SAMP_SIZE; i++)	// mean
	{
		list = head;
		for(k=0; k<list_size; k++)
		{
			assert (list!=NULL); 		// if doesn't fail nor will do after
			assert (list->ns==SAMP_SIZE);
			m[i] += list->samples[i];
			list = list->next;
		}
		assert(list==NULL);
		m[i] /= list_size;
	}
	
	for (i=0; i<SAMP_SIZE; i++)		// std dev
	{	
		list = head;
		for(k=0; k<list_size; k++)
		{
			s[i] += pow( (list->samples[i]-m[i]), 2);
			list = list->next;
		}
		s[i] /= list_size;
	}
	
	// apply
	for (i=0; i<SAMP_SIZE; i++)
	{
		list = head;
		for(k=0; k<list_size; k++)
		{
			if (s[i] > 0)
			{
				dbg_printf("Mean is: %f; while std dev is: %f\n", m[i], s[i]);
				dbg_printf("Doing (%f -%f) / %f\n", list->samples[i], m[i], s[i]);
				list->samples[i] = (list->samples[i] - m[i]) / sqrt(s[i]);
				dbg_printf("Normalized [%d][%d] %f\n\n", i, k, list->samples[i]);
				list = list->next;
			}
		}
	}	

}

//-------------------------------------------------------------------------------------
// PRINT ERROR OVER OUTPUT FILE FOR PLOTTING
// X: epoch or batch
// Y: modulus of the error vector
//-------------------------------------------------------------------------------------
int print_exerr (FILE *fd, int x, float error)
{
	int err = 0;
	
	if ((err = fprintf(fd, "%d,%f\n", x, error)) < 0)
	{
		fprintf(stderr, "Error while appending: %s\n", strerror(err));
		return err;
	}
	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------
// TRAINING
//   -> Stochastic Gradient Descent, Minibatch algorithm
//	 -> Batch error and epoch error is saved on files for future plotting
//------------------------------------------------------------------------------------------------------------------------------
void p_net_train_SGD (	p_net* net, int epochs, int batches, float lrate, float mom, float min_err,
						struct example** tset, int tsize, char* lf, char* gf)
{
	float***	DeltaWji; 		// "matrix" of delta rules to be applyed at the end of each batch
	float** 	DeltaBias;		// "matrix" of delta rule to be applied on bias for each neuron
	float***	oldDeltaWji;	// same thing as DeltaWji but to store the previous step;
	float 		global_err;		// error evalueted on the epoch
	float		local_err; 		// error evaluated on the minibatch
	FILE*		fg = NULL;		// streams for writing on files the global/local error obtained
	FILE*		fl = NULL;
	int 		epc , mbc, smp;	// iterators over EPOCH, MINIBATCH and SAMPLE into minibatch
	int 		l, j, i;
	
	int 		samples_per_batch = tsize / batches;
	int			samples_surplus	= tsize % batches;
	assert (samples_per_batch != 0);
	
	// prepare the DeltaWji and DeltaBias "matrices" for averaging on the batch 
	DeltaWji  	= (float***) malloc (sizeof(float**) * net->nlayers);
	oldDeltaWji = (float***) malloc (sizeof(float**) * net->nlayers);
	DeltaBias 	= (float**) malloc (sizeof(float**) * net->nlayers);
	for (l=0; l<net->nlayers; l++)
	{
		DeltaWji[l]  	= (float**) malloc (sizeof(float*) * net->layers[l].nperc);
		oldDeltaWji[l]  = (float**) malloc (sizeof(float*) * net->layers[l].nperc);		
		DeltaBias[l] 	= (float*) malloc (sizeof(float) * net->layers[l].nperc);
		for (j=0; j<net->layers[l].nperc; j++)
		{
			DeltaWji[l][j] 		= (float*) malloc (sizeof(float) * net->layers[l].perceptrons[j].nweights);
			oldDeltaWji[l][j] 	= (float*) malloc (sizeof(float) * net->layers[l].perceptrons[j].nweights);
			for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
				oldDeltaWji[l][j][i] = DeltaWji[l][j][i] = 0;
			DeltaBias[l][j] = 0;
		}
	} 

	// open streams
	fg = fopen(gf, "a+"); assert(fg!=NULL);
	fl = fopen(lf, "a+"); assert(fl!=NULL);
	epc = 0;
	do {
		// shuffle tset 
		*tset = shuffle_examples(*tset, tsize);
		global_err = 0;
		epc ++;
		printf("EPOCH %d\n", epc );
		// reset delta rule
		for (l=0; l<net->nlayers;l++)
			for (j=0; j<net->layers[l].nperc; j++)
				for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
					oldDeltaWji[l][j][i] = 0;

		//train over the minibatch	
		for (mbc=0; mbc<batches; mbc++)
		{
			// variables to zero
			local_err = 0;
			for (l=0; l<net->nlayers;l++)
				for (j=0; j<net->layers[l].nperc; j++)
					for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
						DeltaWji[l][j][i] = 0;
			// for each sample in the minibatch
			for (smp=0; smp<samples_per_batch; smp++)
			{
				// get element at index mbc*samples_per_batch + smp
				dbg_printf(" Training on example %d\n", mbc*samples_per_batch+smp);
				struct example* ex = get_example(*tset, (mbc*samples_per_batch+smp));
				assert(ex!=NULL);
				local_err += p_net_train_on_example(net, ex, DeltaWji, DeltaBias, lrate);		
			}
			
			// at the end of the batch, average weights and update them
			for (l=0; l<net->nlayers; l++)
			{
				for (j=0; j<net->layers[l].nperc; j++)
				{
					for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
					{
						dbg_printf("  Accumulated DWji[%d][%d][%d]: %f += ", l,j,i, DeltaWji[l][j][i]);
						DeltaWji[l][j][i] /= samples_per_batch;
						dbg_printf("w[%d][%d][%d]: %f -> ", l, j, i, net->layers[l].perceptrons[j].weights[i]);
						net->layers[l].perceptrons[j].weights[i] += DeltaWji[l][j][i] + mom*oldDeltaWji[l][j][i];
						dbg_printf("new w[%d][%d][%d]: %f ", l, j, i, net->layers[l].perceptrons[j].weights[i]);
						dbg_printf("(applied Dwji: %f)\n", DeltaWji[l][j][i]);
						dbg_printf("  mom*oldDeltaWji: %f * %f = %f\n",mom,oldDeltaWji[l][j][i],mom*oldDeltaWji[l][j][i]);
						oldDeltaWji[l][j][i] = DeltaWji[l][j][i] + mom*oldDeltaWji[l][j][i];
						dbg_printf("  oldDeltaWji[%d][%d][%d] is now: %f (sample %d)\n",l,j,i,oldDeltaWji[l][j][i],mbc*samples_per_batch+smp);
					}
					DeltaBias[l][j] /= samples_per_batch;
					net->layers[l].perceptrons[j].bias += DeltaBias[l][j];
					dbg_printf("  new bias [%d][%d] is: %f ", l,j, net->layers[l].perceptrons[j].bias);
					dbg_printf("(applied DeltaBias: %f)\n", DeltaBias[l][j]);
				}
			}
			// add to global
			global_err += local_err; 
			// print the local error averaged on batch
			local_err /= samples_per_batch;
			print_exerr(fl, (epc-1)*batches+mbc, local_err);
			printf(" Batch %d...\n\n", (mbc+1));
			fflush(stdout);
		}
		// mbc == batches
		// the same computation must be done on the surplus samples, if predent		
		if (samples_surplus > 0)
		{
			// variables to zero
			local_err = 0;
			for (l=0; l<net->nlayers;l++)
			{
				for (j=0; j<net->layers[l].nperc; j++)
				{
					for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
						oldDeltaWji[l][j][i] = DeltaWji[l][j][i] = 0;
					DeltaBias[l][j] = 0;
				}
			}
			for (smp=0; smp < samples_surplus; smp++)
			{
				struct example* ex = get_example(*tset, (mbc*samples_per_batch+smp));
				assert(ex!=NULL);
				print_example_label(ex);
				local_err += p_net_train_on_example(net, ex, DeltaWji, DeltaBias, lrate);		
			}
			// at the end of the batch, average weights and update them
			for (l=0; l<net->nlayers; l++)
			{
				for (j=0; j<net->layers[l].nperc; j++)
				{
					for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
					{
						dbg_printf("  Accumulated DWji[%d][%d][%d]: %f -> ", l,j,i, DeltaWji[l][j][i]);
						DeltaWji[l][j][i] /= samples_surplus;
						dbg_printf("w[%d][%d][%d]: %f ->", l, j, i, net->layers[l].perceptrons[j].weights[i]);
						net->layers[l].perceptrons[j].weights[i] += DeltaWji[l][j][i] + mom*oldDeltaWji[l][j][i];
						dbg_printf("new w[%d][%d][%d]: %f ", l, j, i, net->layers[l].perceptrons[j].weights[i]);
						dbg_printf("(applied Dwji: %f)\n", DeltaWji[l][j][i]);
						dbg_printf("  mom*oldDeltaWji: %f * %f = %f\n",mom,oldDeltaWji[l][j][i],mom*oldDeltaWji[l][j][i]);
						oldDeltaWji[l][j][i] = DeltaWji[l][j][i] + mom*oldDeltaWji[l][j][i];
						dbg_printf("  oldDeltaWji[%d][%d][%d] is now: %f (sample %d)\n",l,j,i,oldDeltaWji[l][j][i],mbc*samples_per_batch+smp);
					}
					DeltaBias[l][j] /= samples_surplus;
					net->layers[l].perceptrons[j].bias += DeltaBias[l][j];
					dbg_printf("  new bias [%d][%d] is: %f\n", l,j, net->layers[l].perceptrons[j].bias);
					dbg_printf("(applied DeltaBias: %f)\n", DeltaBias[l][j]);
				}
			}
			// add to global
			global_err += local_err; 
			// print the local error averaged on batch
			local_err /= samples_surplus;
			print_exerr(fl, (epc-1)*batches+mbc, local_err);
		}
		
		
		// average the global error on batches (averaging on each samples has already been done)
		global_err /= tsize; 
		dbg_printf(" Global error at the end of epoch %d is: %f\n\n", epc, global_err);
		print_exerr(fg, epc, global_err);
	}
	while (epc  < epochs  && global_err > min_err);
	
	printf("Stopped after %d epochs with an error of %f\n", epc , global_err);
	fclose(fg); fclose(fl);
	// destroy DeltaWji and DeltaBias
	for (l=0; l<net->nlayers; l++)
	{
		for (j=0; j<net->layers[l].nperc; j++)
		{
			free(DeltaWji[l][j]);
			free(oldDeltaWji[l][j]);
		}
		free(DeltaWji[l]);
		free(oldDeltaWji[l]);
		free(DeltaBias[l]);
	}
	free(DeltaWji);
	free(oldDeltaWji);
	free(DeltaBias);
}

// 23-05 remove moment from here, push to train_SGD
float p_net_train_on_example (p_net* net, struct example* ex, float*** Dw, float** Db, float lrate/*, float mom*/)
{	
	float	local_err = 0; // error evaluated on batch
	int		l, j, i;
	
	predict(net, ex->samples, ex->ns);
/*	print_last_prediction(net);*/
	
	// evaluate now the error over that sample
	local_err += quadratic_error (net, ex->label, ex->nl); 
	// evaluate weight update using backpropagation
	float*** newDeltaWji  = backpropagation_delta (net, ex->label, ex->nl, ex->samples, ex->ns, lrate);
	float**  newDeltaBias = backpropagation_bias (net, lrate);
	// sum the DeltaWji
	for (l=0; l<net->nlayers; l++)
	{
		for (j=0; j<net->layers[l].nperc; j++)
		{
			Db[l][j] += newDeltaBias[l][j];
			for (i=0; i<net->layers[l].perceptrons[j].nweights; i++)
			{
				// ATTENTION!!! It was Dw += new + Dw*perc, so I was getting Dw = Dw + Dw*perc + new
				// this means Dw = (1+perc)*Dw + new!!!
				Dw[l][j][i] = newDeltaWji[l][j][i]; //  + mom * Dw[l][j][i]; 
				dbg_printf ("   DeltaW[%d][%d][%d] with the new is: %f\n",l,j,i,Dw[l][j][i]);
			}
			free(newDeltaWji[l][j]);
		}
		free(newDeltaWji[l]);
		free(newDeltaBias[l]);
	}
	free(newDeltaWji);	
	free(newDeltaBias);
	
	return local_err;
}

