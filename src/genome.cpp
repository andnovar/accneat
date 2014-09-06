/*
 Copyright 2001 The University of Texas at Austin

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "genome.h"

#include <assert.h>
#include <iostream>
#include <cmath>
#include <sstream>

using namespace NEAT;
using std::vector;

Genome::Genome(int id, std::vector<Trait*> t, std::vector<NNode*> n, std::vector<Gene*> g) {
	genome_id=id;
	traits=t;
	nodes=n; 
	genes=g;
}


Genome::Genome(int id, std::vector<Trait*> t, std::vector<NNode*> n, std::vector<Link*> links) {
	std::vector<Link*>::iterator curlink;
	Gene *tempgene;
	traits=t;
	nodes=n;

	genome_id=id;

	//We go through the links and turn them into original genes
	for(curlink=links.begin();curlink!=links.end();++curlink) {
		//Create genes one at a time
		tempgene=new Gene((*curlink)->get_trait_id(), (*curlink)->weight,(*curlink)->in_node,(*curlink)->out_node,(*curlink)->is_recurrent,1.0,0.0);
		genes.push_back(tempgene);
	}

}

Genome::Genome(const Genome& genome)
{
	genome_id = genome.genome_id;

	std::vector<Trait*>::const_iterator curtrait;
	std::vector<NNode*>::const_iterator curnode;
	std::vector<Gene*>::const_iterator curgene;

	for(curtrait=genome.traits.begin(); curtrait!=genome.traits.end(); ++curtrait) {
		traits.push_back(new Trait(**curtrait));
	}

	//Duplicate NNodes
	for(curnode=genome.nodes.begin();curnode!=genome.nodes.end();++curnode) {
		NNode* newnode=new NNode(*curnode);

		(*curnode)->dup=newnode;  //Remember this node's old copy
		//    (*curnode)->activation_count=55;
		nodes.push_back(newnode);    
	}

	NNode *inode; //For forming a gene 
	NNode *onode; //For forming a gene

	//Duplicate Genes
	for(curgene=genome.genes.begin(); curgene!=genome.genes.end(); ++curgene) {
		//First find the nodes connected by the gene's link

		inode=(((*curgene)->lnk)->in_node)->dup;
		onode=(((*curgene)->lnk)->out_node)->dup;

		Gene* newgene=new Gene(*curgene,(*curgene)->lnk->get_trait_id(),inode,onode);
		genes.push_back(newgene);

	}
}

Genome::Genome(int id, std::ifstream &iFile) {

	char curword[128];  //max word size of 128 characters
	char curline[1024]; //max line size of 1024 characters
	char delimiters[] = " \n";

	int done=0;

	//int pause;

	genome_id=id;

	iFile.getline(curline, sizeof(curline));
	int wordcount = NEAT::getUnitCount(curline, delimiters);
	int curwordnum = 0;

	//Loop until file is finished, parsing each line
	while (!done) {

        //std::cout << curline << std::endl;

		if (curwordnum > wordcount || wordcount == 0) {
			iFile.getline(curline, sizeof(curline));
			wordcount = NEAT::getUnitCount(curline, delimiters);
			curwordnum = 0;
		}
        
        std::stringstream ss(curline);
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
        ss >> curword;

		//printf(curword);
		//printf(" test\n");
		//Check for end of Genome
		if (strcmp(curword,"genomeend")==0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
            ss >> curword;
			int idcheck = atoi(curword);
			//iFile>>idcheck;
			if (idcheck!=genome_id) printf("ERROR: id mismatch in genome");
			done=1;
		}

		//Ignore genomestart if it hasn't been gobbled yet
		else if (strcmp(curword,"genomestart")==0) {
			++curwordnum;
			//cout<<"genomestart"<<endl;
		}

		//Ignore comments surrounded by - they get printed to screen
		else if (strcmp(curword,"/*")==0) {
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
            ss >> curword;
			while (strcmp(curword,"*/")!=0) {
				//cout<<curword<<" ";
				//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
                ss >> curword;
			}
			//cout<<endl;
		}

		//Read in a trait
		else if (strcmp(curword,"trait")==0) {
			Trait *newtrait;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));

			curwordnum = wordcount + 1;

            ss.getline(argline, 1024);
			//Allocate the new trait
			newtrait=new Trait(argline);

			//Add trait to vector of traits
			traits.push_back(newtrait);
		}

		//Read in a node
		else if (strcmp(curword,"node")==0) {
			NNode *newnode;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;
            
            ss.getline(argline, 1024);
			//Allocate the new node
			newnode=new NNode(argline,traits);

			//Add the node to the list of nodes
			nodes.push_back(newnode);
		}

		//Read in a Gene
		else if (strcmp(curword,"gene")==0) {
			Gene *newgene;

			char argline[1024];
			//strcpy(argline, NEAT::getUnits(curline, curwordnum, wordcount, delimiters));
			curwordnum = wordcount + 1;

            ss.getline(argline, 1024);
            //std::cout << "New gene: " << ss.str() << std::endl;
			//Allocate the new Gene
            newgene=new Gene(argline,traits,nodes);

			//Add the gene to the genome
			genes.push_back(newgene);

            //std::cout<<"Added gene " << newgene << std::endl;
		}

	}

}

Genome* Genome::new_Genome_load(char *filename) {
	Genome *newgenome;

	int id;

	//char curline[1024];
	char curword[20];  //max word size of 20 characters
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	std::ifstream iFile(filename);

	//Make sure it worked
	//if (!iFile) {
	//	cerr<<"Can't open "<<filename<<" for input"<<endl;
	//	return 0;
	//}

	iFile>>curword;
	//iFile.getline(curline, sizeof(curline));
	//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));

	//Bypass initial comment
	if (strcmp(curword,"/*")==0) {
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
		iFile>>curword;
		while (strcmp(curword,"*/")!=0) {
			printf("%s ",curword);
			//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
			iFile>>curword;
		}

		//cout<<endl;
		iFile>>curword;
		//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
	}

	//strcpy(curword, NEAT::getUnit(curline, curwordnum++, delimiters));
	//id = atoi(curword);
	iFile>>id;

	newgenome=new Genome(id,iFile);

	iFile.close();

	return newgenome;
}

Genome::~Genome() {
	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	for(curtrait=traits.begin();curtrait!=traits.end();++curtrait) {
		delete (*curtrait);
	}

	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		delete (*curnode);
	}

	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		delete (*curgene);
	}

}

Network *Genome::genesis(int id) {
	std::vector<NNode*>::iterator curnode; 
	std::vector<Gene*>::iterator curgene;
	NNode *newnode;
	Link *curlink;

	double maxweight=0.0; //Compute the maximum weight for adaptation purposes
	double weight_mag; //Measures absolute value of weights

	//Inputs and outputs will be collected here for the network
	//All nodes are collected in an all_list- 
	//this will be used for later safe destruction of the net
	std::vector<NNode*> inlist;
	std::vector<NNode*> outlist;
	std::vector<NNode*> all_list;

	//Gene translation variables
	NNode *inode;
	NNode *onode;

	//The new network
	Network *newnet;

	//Create the nodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		newnode=new NNode((*curnode)->type,(*curnode)->node_id);

		//Derive the node parameters from the trait pointed to
		newnode->derive_trait( get_trait(*curnode) );

		//Check for input or output designation of node
		if (((*curnode)->gen_node_label)==INPUT) 
			inlist.push_back(newnode);
		if (((*curnode)->gen_node_label)==BIAS) 
			inlist.push_back(newnode);
		if (((*curnode)->gen_node_label)==OUTPUT)
			outlist.push_back(newnode);

		//Keep track of all nodes, not just input and output
		all_list.push_back(newnode);

		//Have the node specifier point to the node it generated
		(*curnode)->analogue=newnode;

	}

	//Create the links by iterating through the genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		//Only create the link if the gene is enabled
		if (((*curgene)->enable)==true) {
			curlink=(*curgene)->lnk;
			inode=(curlink->in_node)->analogue;
			onode=(curlink->out_node)->analogue;

			//NOTE: This line could be run through a recurrency check if desired
			// (no need to in the current implementation of NEAT)
			Link newlink(curlink->weight,inode,onode,curlink->is_recurrent);

			(onode->incoming).push_back(newlink);

			//Derive link's parameters from its Trait pointer
			newlink.derive_trait( get_trait(curlink) );

			//Keep track of maximum weight
			if (newlink.weight>0)
				weight_mag=newlink.weight;
			else weight_mag=-newlink.weight;
			if (weight_mag>maxweight)
				maxweight=weight_mag;
		}
	}

	//Create the new network
	newnet = new Network(inlist, outlist, all_list, id, false, maxweight);

	//Attach genotype and phenotype together
	phenotype=newnet;

	return newnet;

}

bool Genome::verify() {
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;
	std::vector<Gene*>::iterator curgene2;
	NNode *inode;
	NNode *onode;

	bool disab;

	int last_id;

	//int pause;

	//cout<<"Verifying Genome id: "<<this->genome_id<<endl;

	if (this==0) {
		//cout<<"ERROR GENOME EMPTY"<<endl;
		//cin>>pause;
	}

	//Check each gene's nodes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		inode=((*curgene)->lnk)->in_node;
		onode=((*curgene)->lnk)->out_node;

		//Look for inode
		curnode=nodes.begin();
		while((curnode!=nodes.end())&&
			((*curnode)!=inode))
			++curnode;

		if (curnode==nodes.end()) {
			//cout<<"MISSING iNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
			//cin>>pause;
			return false;
		}

		//Look for onode
		curnode=nodes.begin();
		while((curnode!=nodes.end())&&
			((*curnode)!=onode))
			++curnode;

		if (curnode==nodes.end()) {
			//cout<<"MISSING oNODE FROM GENE NOT IN NODES OF GENOME!!"<<endl;
			//cin>>pause;
			return false;
		}

	}

	//Check for NNodes being out of order
	last_id=0;
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		if ((*curnode)->node_id<last_id) {
			//cout<<"ALERT: NODES OUT OF ORDER in "<<this<<endl;
			//cin>>pause;
			return false;
		}

		last_id=(*curnode)->node_id;
	}


	//Make sure there are no duplicate genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {

		for(curgene2=genes.begin();curgene2!=genes.end();++curgene2) {
			if (((*curgene)!=(*curgene2))&&
				((((*curgene)->lnk)->is_recurrent)==(((*curgene2)->lnk)->is_recurrent))&&
				((((((*curgene2)->lnk)->in_node)->node_id)==((((*curgene)->lnk)->in_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((*curgene)->lnk)->out_node)->node_id)))) {
					//cout<<"ALERT: DUPLICATE GENES: "<<(*curgene)<<(*curgene2)<<endl;
					//cout<<"INSIDE GENOME: "<<this<<endl;

					//cin>>pause;
				}


		}
	}

	//See if a gene is not disabled properly
	//Note this check does not necessary mean anything is wrong
	//
	//if (nodes.size()>=15) {
	//disab=false;
	////Go through genes and see if one is disabled
	//for(curgene=genes.begin();curgene!=genes.end();++curgene) {
	//if (((*curgene)->enable)==false) disab=true;
	//}

	//if (disab==false) {
	//cout<<"ALERT: NO DISABLED GENE IN GENOME: "<<this<<endl;
	////cin>>pause;
	//}

	//}
	//

	//Check for 2 disables in a row
	//Note:  Again, this is not necessarily a bad sign
	if (nodes.size()>=500) {
		disab=false;
		for(curgene=genes.begin();curgene!=genes.end();++curgene) {
			if ((((*curgene)->enable)==false)&&(disab==true)) {
				//cout<<"ALERT: 2 DISABLES IN A ROW: "<<this<<endl;
			}
			if (((*curgene)->enable)==false) disab=true;
			else disab=false;
		}
	}

	//cout<<"GENOME OK!"<<endl;

	return true;
}


//Print the genome to a file
void Genome::print_to_file(std::ofstream &outFile) {
  std::vector<Trait*>::iterator curtrait;
  std::vector<NNode*>::iterator curnode;
  std::vector<Gene*>::iterator curgene;

  outFile<<"genomestart "<<genome_id<<std::endl;

  //Output the traits
  for(curtrait=traits.begin();curtrait!=traits.end();++curtrait) {
    (*curtrait)->trait_id=curtrait-traits.begin()+1;
    (*curtrait)->print_to_file(outFile);
  }

  //Output the nodes
  for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
    (*curnode)->print_to_file(outFile);
  }

  //Output the genes
  for(curgene=genes.begin();curgene!=genes.end();++curgene) {
    (*curgene)->print_to_file(outFile);
  }

  outFile<<"genomeend "<<genome_id<<std::endl;

}


void Genome::print_to_file(std::ostream &outFile) {
	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	//char tempbuf[128];
	//sprintf(tempbuf, "genomestart %d\n", genome_id);
	//outFile.write(strlen(tempbuf), tempbuf);
    outFile<<"genomestart "<<genome_id<<std::endl;

	//Output the traits
	for(curtrait=traits.begin();curtrait!=traits.end();++curtrait) {
		(*curtrait)->trait_id=curtrait-traits.begin()+1;
		(*curtrait)->print_to_file(outFile);
	}

	//Output the nodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		(*curnode)->print_to_file(outFile);
	}

	//Output the genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		(*curgene)->print_to_file(outFile);
	}

	//char tempbuf2[128];
	//sprintf(tempbuf2, sizeof(tempbuf2), "genomeend %d\n", genome_id);
	//outFile.write(strlen(tempbuf2), tempbuf2);
    outFile << "genomeend " << genome_id << std::endl << std::endl << std::endl;
	//char tempbuf4[1024];
	//sprintf(tempbuf4, sizeof(tempbuf4), "\n\n");
	//outFile.write(strlen(tempbuf4), tempbuf4);
}

void Genome::print_to_filename(char *filename) {
	std::ofstream oFile(filename);
	//oFile.open(filename, std::ostream::Write);
	print_to_file(oFile);
	oFile.close();
}

int Genome::get_last_node_id() {
	return ((*(nodes.end() - 1))->node_id)+1;
}

double Genome::get_last_gene_innovnum() {
	return ((*(genes.end() - 1))->innovation_num)+1;
}

Genome *Genome::duplicate(int new_id) {
	//Collections for the new Genome
	std::vector<Trait*> traits_dup;
	std::vector<NNode*> nodes_dup;
	std::vector<Gene*> genes_dup;

	//Iterators for the old Genome
	std::vector<Trait*>::iterator curtrait;
	std::vector<NNode*>::iterator curnode;
	std::vector<Gene*>::iterator curgene;

	//New item pointers
	Trait *newtrait;
	NNode *newnode;
	Gene *newgene;
	NNode *inode; //For forming a gene 
	NNode *onode; //For forming a gene

	Genome *newgenome;

	//verify();

	//Duplicate the traits
	for(curtrait=traits.begin();curtrait!=traits.end();++curtrait) {
		newtrait=new Trait(*curtrait);
		traits_dup.push_back(newtrait);
	}

	//Duplicate NNodes
	for(curnode=nodes.begin();curnode!=nodes.end();++curnode) {
		newnode=new NNode(*curnode);

		(*curnode)->dup=newnode;  //Remember this node's old copy
		//    (*curnode)->activation_count=55;
		nodes_dup.push_back(newnode);    
	}

	//Duplicate Genes
	for(curgene=genes.begin();curgene!=genes.end();++curgene) {
		//First find the nodes connected by the gene's link

		inode=(((*curgene)->lnk)->in_node)->dup;
		onode=(((*curgene)->lnk)->out_node)->dup;

		newgene=new Gene(*curgene,(*curgene)->lnk->get_trait_id(),inode,onode);
		genes_dup.push_back(newgene);

	}

	//Finally, return the genome
	newgenome=new Genome(new_id,traits_dup,nodes_dup,genes_dup);

	return newgenome;

}

void Genome::mutate_random_trait() {
    traits[ randint(0,(traits.size())-1) ]->mutate();
	//TRACK INNOVATION? (future possibility)
}

void Genome::mutate_link_trait(int times) {
    for(int i = 0; i < times; i++) {
        Trait *trait = traits[ randint(0,(traits.size())-1) ];
        Gene *gene = genes[ randint(0,genes.size()-1) ];
        
        if(!gene->frozen) {
            gene->lnk->set_trait_id(trait->trait_id);
        }
    }
}

void Genome::mutate_node_trait(int times) {
    for(int i = 0; i < times; i++) {
        Trait *trait = traits[ randint(0,(traits.size())-1) ];
        NNode *node = nodes[ randint(0,nodes.size()-1) ];

        if(!node->frozen) {
            node->set_trait_id(trait->trait_id);
        }
    }

    //TRACK INNOVATION! - possible future use
    //for any gene involving the mutated node, perturb that gene's
    //mutation number
}

void Genome::mutate_link_weights(double power,double rate,mutator mut_type) {
	//Go through all the Genes and perturb their link's weights

	double num = 0.0; //counts gene placement
	double gene_total = (double)genes.size();
	double endpart = gene_total*0.8; //Signifies the last part of the genome
	double powermod = 1.0; //Modified power by gene number
	//The power of mutation will rise farther into the genome
	//on the theory that the older genes are more fit since
	//they have stood the test of time

	bool severe = randfloat() > 0.5;  //Once in a while really shake things up

	//Loop on all genes  (ORIGINAL METHOD)
	for(Gene *gene: genes) {

		//The following if determines the probabilities of doing cold gaussian
		//mutation, meaning the probability of replacing a link weight with
		//another, entirely random weight.  It is meant to bias such mutations
		//to the tail of a genome, because that is where less time-tested genes
		//reside.  The gausspoint and coldgausspoint represent values above
		//which a random float will signify that kind of mutation.  

		//Don't mutate weights of frozen links
		if (!(gene->frozen)) {
            double gausspoint;
            double coldgausspoint;

			if (severe) {
				gausspoint=0.3;
				coldgausspoint=0.1;
			}
			else if ((gene_total>=10.0)&&(num>endpart)) {
				gausspoint=0.5;  //Mutate by modification % of connections
				coldgausspoint=0.3; //Mutate the rest by replacement % of the time
			}
			else {
				//Half the time don't do any cold mutations
				if (randfloat()>0.5) {
					gausspoint=1.0-rate;
					coldgausspoint=1.0-rate-0.1;
				}
				else {
					gausspoint=1.0-rate;
					coldgausspoint=1.0-rate;
				}
			}

			//Possible methods of setting the perturbation:
			double randnum = randposneg()*randfloat()*power*powermod;
			if (mut_type==GAUSSIAN) {
				double randchoice = randfloat();
				if (randchoice > gausspoint)
					(gene->lnk)->weight+=randnum;
				else if (randchoice > coldgausspoint)
					(gene->lnk)->weight=randnum;
			}
			else if (mut_type==COLDGAUSSIAN)
				(gene->lnk)->weight=randnum;

			//Cap the weights at 8.0 (experimental)
			if ((gene->lnk)->weight > 8.0) (gene->lnk)->weight = 8.0;
			else if ((gene->lnk)->weight < -8.0) (gene->lnk)->weight = -8.0;

			//Record the innovation
			gene->mutation_num = (gene->lnk)->weight;

			num+=1.0;
		}

	} //end for loop
}

void Genome::mutate_toggle_enable(int times) {
    for(int i = 0; i < times; i++) {
        Gene *gene = genes[ randint(0,genes.size()-1) ];

        if(!gene->enable) {
            gene->enable = true;
        } else {
			//We need to make sure that another gene connects out of the in-node
			//Because if not a section of network will break off and become isolated
            bool found = false;
            for(Gene *checkgene: genes) {
                if( (checkgene->lnk->in_node == gene->lnk->in_node)
                    && checkgene->enable
                    && (checkgene->innovation_num != gene->innovation_num) ) {
                    found = true;
                    break;
                }
            }

			//Disable the gene if it's safe to do so
			if(found)
				gene->enable = false;
        }
    }
}

void Genome::mutate_gene_reenable() {
	//Search for a disabled gene
    for(Gene *g: genes) {
        if(!g->enable) {
            g->enable = true;
            break;
        }
    }
}

bool Genome::mutate_add_node(std::vector<Innovation*> &innovs,
                             int &curnode_id,
                             double &curinnov) {
	NNode *in_node; //Here are the nodes connected by the gene
	NNode *out_node; 
	Link *thelink;  //The link inside the random gene

	std::vector<Innovation*>::iterator theinnov; //For finding a historical match
	bool done=false;

	Gene *newgene1;  //The new Genes
	Gene *newgene2;
	NNode *newnode;   //The new NNode
	//double splitweight;  //If used, Set to sqrt(oldweight of oldlink)
	double oldweight;  //The weight of the original link

    Gene *thegene = nullptr;
    for(int i = 0; !thegene && i < 20; i++) {
        Gene *g = genes[ randint(0,genes.size()-1) ];
        //If either the gene is disabled, or it has a bias input, try again
        if( g->enable && (g->lnk->in_node->gen_node_label != BIAS) )
            thegene = g;
    }
	//If we couldn't find anything so say goodbye
	if (!thegene) {
		return false;
    }

	//Disabled the gene
	thegene->enable=false;

	//Extract the link
	thelink=thegene->lnk;
	oldweight=thegene->lnk->weight;

	//Extract the nodes
	in_node=thelink->in_node;
	out_node=thelink->out_node;

	//Check to see if this innovation has already been done   
	//in another genome
	//Innovations are used to make sure the same innovation in
	//two separate genomes in the same generation receives
	//the same innovation number.
	theinnov=innovs.begin();

	while(!done) {

		if (theinnov==innovs.end()) {
			//The innovation is totally novel

			//Get the old link's trait
            int trait_id = thelink->get_trait_id();

			//Create the new NNode
			//By convention, it will point to the first trait
			newnode=new NNode(NEURON,curnode_id++,HIDDEN);
			newnode->set_trait_id((*(traits.begin()))->trait_id);

			//Create the new Genes
			if (thelink->is_recurrent) {
				newgene1=new Gene(trait_id,1.0,in_node,newnode,true,curinnov,0);
				newgene2=new Gene(trait_id,oldweight,newnode,out_node,false,curinnov+1,0);
				curinnov+=2.0;
			}
			else {
				newgene1=new Gene(trait_id,1.0,in_node,newnode,false,curinnov,0);
				newgene2=new Gene(trait_id,oldweight,newnode,out_node,false,curinnov+1,0);
				curinnov+=2.0;
			}

			//Add the innovations (remember what was done)
			innovs.push_back(new Innovation(in_node->node_id,out_node->node_id,curinnov-2.0,curinnov-1.0,newnode->node_id,thegene->innovation_num));      

			done=true;
		}

		// We check to see if an innovation already occured that was:
		//   -A new node
		//   -Stuck between the same nodes as were chosen for this mutation
		//   -Splitting the same gene as chosen for this mutation 
		//   If so, we know this mutation is not a novel innovation
		//   in this generation
		//   so we make it match the original, identical mutation which occured
		//   elsewhere in the population by coincidence 
		else if (((*theinnov)->innovation_type==NEWNODE)&&
			((*theinnov)->node_in_id==(in_node->node_id))&&
			((*theinnov)->node_out_id==(out_node->node_id))&&
			((*theinnov)->old_innov_num==thegene->innovation_num)) 
		{
			//Here, the innovation has been done before

			//Get the old link's trait
            int trait_id = thelink->get_trait_id();

			//Create the new NNode
			newnode=new NNode(NEURON,(*theinnov)->newnode_id,HIDDEN);      
			//By convention, it will point to the first trait
			//Note: In future may want to change this
			newnode->set_trait_id((*(traits.begin()))->trait_id);

			//Create the new Genes
			if (thelink->is_recurrent) {
				newgene1=new Gene(trait_id,1.0,in_node,newnode,true,(*theinnov)->innovation_num1,0);
				newgene2=new Gene(trait_id,oldweight,newnode,out_node,false,(*theinnov)->innovation_num2,0);
			}
			else {
				newgene1=new Gene(trait_id,1.0,in_node,newnode,false,(*theinnov)->innovation_num1,0);
				newgene2=new Gene(trait_id,oldweight,newnode,out_node,false,(*theinnov)->innovation_num2,0);
			}

			done=true;
		}
		else ++theinnov;
	}

	//Now add the new NNode and new Genes to the Genome
	add_gene(genes,newgene1);  //Add genes in correct order
	add_gene(genes,newgene2);
	node_insert(nodes,newnode);

	return true;

}

bool Genome::mutate_add_link(std::vector<Innovation*> &innovs,
                             double &curinnov,
                             int tries) {

	NNode *in_node = nullptr; //Pointers to the nodes
	NNode *out_node = nullptr; //Pointers to the nodes

	//Decide whether to make this recurrent
	bool do_recur = randfloat() < NEAT::recur_only_prob;

    // Try to find nodes for link.
    {
        bool found_nodes = false;

        //Find the first non-sensor so that the to-node won't look at sensors as
        //possible destinations
        int first_nonsensor = 0;
        for(; nodes[first_nonsensor]->get_type() == SENSOR; first_nonsensor++) {
        }

        for(int trycount = 0; !found_nodes && (trycount < tries); trycount++) {
            //Here is the recurrent finder loop- it is done separately
            if(do_recur) {
                //Some of the time try to make a recur loop
                // todo: make this an NE parm?
                if (randfloat() > 0.5) {
                    in_node = nodes[ randint(first_nonsensor,nodes.size()-1) ];
                    out_node = in_node;
                }
                else {
                    //Choose random nodenums
                    in_node = nodes[ randint(0,nodes.size()-1) ];
                    out_node = nodes[ randint(first_nonsensor,nodes.size()-1) ];
                }
            } else {
                //Choose random nodenums
                in_node = nodes[ randint(0,nodes.size()-1) ];
                out_node = nodes[ randint(first_nonsensor,nodes.size()-1) ];
            }

            found_nodes = !link_exists(in_node, out_node, do_recur)
                && (do_recur == is_recur(in_node, out_node));
        }

        assert( out_node->type != SENSOR );

        //Continue only if an open link was found
        if(!found_nodes) {
            return false;
        }
    }

    // Create the gene.
    {
        Gene *newgene = nullptr;

        // Try to find existing innovation.
        for(Innovation *innov: innovs) {
            if( (innov->innovation_type == NEWLINK) &&
                (innov->node_in_id == in_node->node_id) &&
                (innov->node_out_id == out_node->node_id) &&
                (innov->recur_flag == do_recur)) {

                //Create new gene using existing innovation.
                newgene = new Gene(innov->new_trait_id,
                                   innov->new_weight,
                                   in_node,
                                   out_node,
                                   do_recur,
                                   innov->innovation_num1,
                                   0);
            }
        }

        //The innovation is totally novel
        if(!newgene) {
            //Choose a random trait
            int trait_id = randint(1, (int)traits.size());

            //Choose the new weight
            //newweight=(gaussrand())/1.5;  //Could use a gaussian
            double newweight = randposneg() * randfloat() * 1.0; //used to be 10.0

            //Create the new gene
            newgene = new Gene(trait_id,
                               newweight,
                               in_node,
                               out_node,
                               do_recur,
                               curinnov,
                               newweight);

            //Add the innovation
            innovs.push_back(new Innovation(in_node->node_id,
                                            out_node->node_id,
                                            curinnov,
                                            newweight,
                                            trait_id));
            curinnov += 1.0;
        }

        add_gene(genes,newgene);  //Adds the gene in correct order
    }

    return true;
}

//Adds a new gene that has been created through a mutation in the
//*correct order* into the list of genes in the genome
void Genome::add_gene(std::vector<Gene*> &glist,Gene *g) {
  std::vector<Gene*>::iterator curgene;
  double inum=g->innovation_num;

  //std::cout<<"**ADDING GENE: "<<g->innovation_num<<std::endl;

  curgene=glist.begin();
  while ((curgene!=glist.end())&&
	 (((*curgene)->innovation_num)<inum)) {
    //p1innov=(*curgene)->innovation_num;
    //printf("Innov num: %f",p1innov);  
    ++curgene;

    //Con::printf("looking gene %f", (*curgene)->innovation_num);
  }


  glist.insert(curgene,g);

}


void Genome::node_insert(std::vector<NNode*> &nlist,NNode *n) {
	std::vector<NNode*>::iterator curnode;

	int id=n->node_id;

	curnode=nlist.begin();
	while ((curnode!=nlist.end())&&
		(((*curnode)->node_id)<id)) 
		++curnode;

	nlist.insert(curnode,n);

}

Genome *Genome::mate_multipoint(Genome *g,int genomeid,double fitness1,double fitness2, bool interspec_flag) {
	//The baby Genome will contain these new Traits, NNodes, and Genes
	vector<Trait*> newtraits;
	vector<NNode*> newnodes;   
	vector<Gene*> newgenes;    
	Genome *new_genome;

	std::vector<Gene*>::iterator curgene2;  //Checks for link duplication

	//iterators for moving through the two parents' traits
	std::vector<Trait*>::iterator p1trait;
	std::vector<Trait*>::iterator p2trait;

	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene = nullptr;  //Gene chosen for baby to inherit
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;
	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 

	bool disable;  //Set to true if we want to disabled a chosen gene

	disable=false;
	Gene *newgene;

	bool p1better; //Tells if the first genome (this one) has better fitness or not

	bool skip;

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In the future, may decide on a different method for trait mating
    assert(traits.size() == g->traits.size());
    for(size_t i = 0, n = traits.size(); i < n; i++) {
        newtraits.push_back( new Trait(traits[i], g->traits[i]) );
    }

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	if (fitness1>fitness2) 
		p1better=true;
	else if (fitness1==fitness2) {
		if (genes.size()<(g->genes.size()))
			p1better=true;
		else p1better=false;
	}
	else 
		p1better=false;

	//Make sure all sensors and outputs are included
    for(NNode *node: g->nodes) {
		if( (node->gen_node_label == INPUT)
            || (node->gen_node_label == BIAS)
            || (node->gen_node_label == OUTPUT)) {

            //Create a new node off the sensor or output
            NNode *new_node = new NNode(node);
            //Add the new node
            node_insert(newnodes, new_node);
        }
    }

	//Now move through the Genes of each parent until both genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {


			skip=false;  //Default to not skipping a chosen gene

			if (p1gene==genes.end()) {
				chosengene=*p2gene;
				++p2gene;
				if (p1better) skip=true;  //Skip excess from the worse genome
			}
			else if (p2gene==(g->genes).end()) {
				chosengene=*p1gene;
				++p1gene;
				if (!p1better) skip=true; //Skip excess from the worse genome
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					if (randfloat()<0.5) {
						chosengene=*p1gene;
					}
					else {
						chosengene=*p2gene;
					}

					//If one is disabled, the corresponding gene in the offspring
					//will likely be disabled
					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						if (randfloat()<0.75) disable=true;

					++p1gene;
					++p2gene;

				}
				else if (p1innov<p2innov) {
					chosengene=*p1gene;
					++p1gene;

					if (!p1better) skip=true;

				}
				else if (p2innov<p1innov) {
					chosengene=*p2gene;
					++p2gene;
					if (p1better) skip=true;
				}
			}

			/*
			//Uncomment this line to let growth go faster (from both parents excesses)
			skip=false;

			//For interspecies mating, allow all genes through:
			if (interspec_flag)
				skip=false;
			*/

			//Check to see if the chosengene conflicts with an already chosen gene
			//i.e. do they represent the same link    
			curgene2=newgenes.begin();
			while ((curgene2!=newgenes.end())&&
				(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent)) ))&&
				(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
				(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
				(!((((*curgene2)->lnk)->is_recurrent)))&&
				(!((((chosengene)->lnk)->is_recurrent))) )))
			{	
				++curgene2;
			}

			if (curgene2!=newgenes.end()) skip=true;  //Links conflicts, abort adding

			if (!skip) {

				//Now add the chosengene to the baby

				//Next check for the nodes, add them if not in the baby Genome already
				inode=(chosengene->lnk)->in_node;
				onode=(chosengene->lnk)->out_node;

				//Check for inode in the newnodes list
				if (inode->node_id<onode->node_id) {
					//inode before onode

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;

					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_inode=new NNode(inode);
						node_insert(newnodes,new_inode);

					}
					else {
						new_inode=(*curnode);

					}

					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_onode=new NNode(onode);
						node_insert(newnodes,new_onode);

					}
					else {
						new_onode=(*curnode);
					}

				}
				//If the onode has a higher id than the inode we want to add it first
				else {
					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_onode=new NNode(onode);
						//newnodes.push_back(new_onode);
						node_insert(newnodes,new_onode);

					}
					else {
						new_onode=(*curnode);

					}

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_inode=new NNode(inode);
						node_insert(newnodes,new_inode);
					}
					else {
						new_inode=(*curnode);

					}

				} //End NNode checking section- NNodes are now in new Genome

				//Add the Gene
                newgene=new Gene(chosengene,chosengene->lnk->get_trait_id(),new_inode,new_onode);
				if (disable) {
					newgene->enable=false;
					disable=false;
				}
				newgenes.push_back(newgene);
			}

		}

		new_genome=new Genome(genomeid,newtraits,newnodes,newgenes);

		//Return the baby Genome
		return (new_genome);

}

Genome *Genome::mate_multipoint_avg(Genome *g,int genomeid,double fitness1,double fitness2,bool interspec_flag) {
	//The baby Genome will contain these new Traits, NNodes, and Genes
	std::vector<Trait*> newtraits;
	std::vector<NNode*> newnodes;
	std::vector<Gene*> newgenes;

	//iterators for moving through the two parents' traits
	std::vector<Trait*>::iterator p1trait;
	std::vector<Trait*>::iterator p2trait;
	Trait *newtrait;

	std::vector<Gene*>::iterator curgene2; //Checking for link duplication

	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene = nullptr;  //Gene chosen for baby to inherit
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;

	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 

	//This Gene is used to hold the average of the two genes to be averaged
	Gene *avgene;

	Gene *newgene;

	bool skip;

	bool p1better;  //Designate the better genome

	// BLX-alpha variables - for assigning weights within a good space 
	// This is for BLX-style mating, which isn't used in this implementation,
	//   but can easily be made from multipoint_avg 
	//double blx_alpha;
	//double w1,w2;
	//double blx_min, blx_max;
	//double blx_range;   //The space range
	//double blx_explore;  //Exploration space on left or right
	//double blx_pos;  //Decide where to put gnes distancewise
	//blx_pos=randfloat();

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	//In future, could be done differently
	p2trait=(g->traits).begin();
	for(p1trait=traits.begin();p1trait!=traits.end();++p1trait) {
		newtrait=new Trait(*p1trait,*p2trait);  //Construct by averaging
		newtraits.push_back(newtrait);
		++p2trait;
	}

	//Set up the avgene
	avgene=new Gene(0,0,0,0,0,0,0);

	//NEW 3/17/03 Make sure all sensors and outputs are included
	for(curnode=(g->nodes).begin();curnode!=(g->nodes).end();++curnode) {
		if ((((*curnode)->gen_node_label)==INPUT)||
			(((*curnode)->gen_node_label)==OUTPUT)||
			(((*curnode)->gen_node_label)==BIAS)) {
				//Create a new node off the sensor or output
				new_onode=new NNode((*curnode));

				//Add the new node
				node_insert(newnodes,new_onode);

			}

	}

	//Figure out which genome is better
	//The worse genome should not be allowed to add extra structural baggage
	//If they are the same, use the smaller one's disjoint and excess genes only
	if (fitness1>fitness2) 
		p1better=true;
	else if (fitness1==fitness2) {
		if (genes.size()<(g->genes.size()))
			p1better=true;
		else p1better=false;
	}
	else 
		p1better=false;


	//Now move through the Genes of each parent until both genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {

			avgene->enable=true;  //Default to enabled

			skip=false;

			if (p1gene==genes.end()) {
				chosengene=*p2gene;
				++p2gene;

				if (p1better) skip=true;

			}
			else if (p2gene==(g->genes).end()) {
				chosengene=*p1gene;
				++p1gene;

				if (!p1better) skip=true;
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					//Average them into the avgene
					if (randfloat()>0.5) {
                        avgene->lnk->set_trait_id((*p1gene)->lnk->get_trait_id());
					} else {
                        avgene->lnk->set_trait_id((*p2gene)->lnk->get_trait_id());
                    }

					//WEIGHTS AVERAGED HERE
					(avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;

				

					////BLX-alpha method (Eschelman et al 1993)
					////Not used in this implementation, but the commented code works
					////with alpha=0.5, this will produce babies evenly in exploitation and exploration space
					////and uniformly distributed throughout
					//blx_alpha=-0.4;
					//w1=(((*p1gene)->lnk)->weight);
					//w2=(((*p2gene)->lnk)->weight);
					//if (w1>w2) {
					//blx_max=w1; blx_min=w2;
					//}
					//else {
					//blx_max=w2; blx_min=w1;
					//}
					//blx_range=blx_max-blx_min;
					//blx_explore=blx_alpha*blx_range;
					////Now extend the range into the exploraton space
					//blx_min-=blx_explore;
					//blx_max+=blx_explore;
					//blx_range=blx_max-blx_min;
					////Set the weight in the new range
					//(avgene->lnk)->weight=blx_min+blx_pos*blx_range;
					//

					if (randfloat()>0.5) (avgene->lnk)->in_node=((*p1gene)->lnk)->in_node;
					else (avgene->lnk)->in_node=((*p2gene)->lnk)->in_node;

					if (randfloat()>0.5) (avgene->lnk)->out_node=((*p1gene)->lnk)->out_node;
					else (avgene->lnk)->out_node=((*p2gene)->lnk)->out_node;

					if (randfloat()>0.5) (avgene->lnk)->is_recurrent=((*p1gene)->lnk)->is_recurrent;
					else (avgene->lnk)->is_recurrent=((*p2gene)->lnk)->is_recurrent;

					avgene->innovation_num=(*p1gene)->innovation_num;
					avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						if (randfloat()<0.75) avgene->enable=false;

					chosengene=avgene;
					++p1gene;
					++p2gene;
				}
				else if (p1innov<p2innov) {
					chosengene=*p1gene;
					++p1gene;

					if (!p1better) skip=true;
				}
				else if (p2innov<p1innov) {
					chosengene=*p2gene;
					++p2gene;

					if (p1better) skip=true;
				}
			}

			/*
			//THIS LINE MUST BE DELETED TO SLOW GROWTH
			skip=false;

			//For interspecies mating, allow all genes through:
			if (interspec_flag)
				skip=false;
			*/

			//Check to see if the chosengene conflicts with an already chosen gene
			//i.e. do they represent the same link    
			curgene2=newgenes.begin();
			while ((curgene2!=newgenes.end()))

			{

				if (((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
					(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
					((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent)))||
					((((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
					(((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
					(!((((*curgene2)->lnk)->is_recurrent)))&&
					(!((((chosengene)->lnk)->is_recurrent)))     ))
				{ 
					skip=true;

				}
				++curgene2;
			}

			if (!skip) {

				//Now add the chosengene to the baby

				//Next check for the nodes, add them if not in the baby Genome already
				inode=(chosengene->lnk)->in_node;
				onode=(chosengene->lnk)->out_node;

				//Check for inode in the newnodes list
				if (inode->node_id<onode->node_id) {

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;

					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_inode=new NNode(inode);

						node_insert(newnodes,new_inode);
					}
					else {
						new_inode=(*curnode);

					}

					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_onode=new NNode(onode);

						node_insert(newnodes,new_onode);
					}
					else {
						new_onode=(*curnode);
					}
				}
				//If the onode has a higher id than the inode we want to add it first
				else {
					//Checking for onode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=onode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_onode=new NNode(onode);

						node_insert(newnodes,new_onode);
					}
					else {
						new_onode=(*curnode);
					}

					//Checking for inode's existence
					curnode=newnodes.begin();
					while((curnode!=newnodes.end())&&
						((*curnode)->node_id!=inode->node_id)) 
						++curnode;
					if (curnode==newnodes.end()) {
						//Here we know the node doesn't exist so we have to add it
						new_inode=new NNode(inode);

						node_insert(newnodes,new_inode);
					}
					else {
						new_inode=(*curnode);

					}

				} //End NNode checking section- NNodes are now in new Genome

				//Add the Gene
				newgene=new Gene(chosengene,chosengene->lnk->get_trait_id(),new_inode,new_onode);

				newgenes.push_back(newgene);

			}  //End if which checked for link duplicationb

		}

		delete avgene;  //Clean up used object

		//Return the baby Genome
		return (new Genome(genomeid,newtraits,newnodes,newgenes));

}

Genome *Genome::mate_singlepoint(Genome *g,int genomeid) {
	//The baby Genome will contain these new Traits, NNodes, and Genes
	std::vector<Trait*> newtraits; 
	std::vector<NNode*> newnodes;   
	std::vector<Gene*> newgenes;    

	//iterators for moving through the two parents' traits
	std::vector<Trait*>::iterator p1trait;
	std::vector<Trait*>::iterator p2trait;
	Trait *newtrait;

	std::vector<Gene*>::iterator curgene2;  //Check for link duplication

	//iterators for moving through the two parents' genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;
	std::vector<Gene*>::iterator stopper;  //To tell when finished
	std::vector<Gene*>::iterator p2stop;
	std::vector<Gene*>::iterator p1stop;
	double p1innov;  //Innovation numbers for genes inside parents' Genomes
	double p2innov;
	Gene *chosengene = nullptr;  //Gene chosen for baby to inherit
	NNode *inode;  //NNodes connected to the chosen Gene
	NNode *onode;
	NNode *new_inode;
	NNode *new_onode;
	std::vector<NNode*>::iterator curnode;  //For checking if NNodes exist already 

	//This Gene is used to hold the average of the two genes to be averaged
	Gene *avgene;

	int crosspoint; //The point in the Genome to cross at
	int genecounter; //Counts up to the crosspoint
	bool skip; //Used for skipping unwanted genes

	//First, average the Traits from the 2 parents to form the baby's Traits
	//It is assumed that trait lists are the same length
	p2trait=(g->traits).begin();
	for(p1trait=traits.begin();p1trait!=traits.end();++p1trait) {
		newtrait=new Trait(*p1trait,*p2trait);  //Construct by averaging
		newtraits.push_back(newtrait);
		++p2trait;
	}

	//Set up the avgene
	avgene=new Gene(0,0,0,0,0,0,0);

	//Decide where to cross  (p1gene will always be in smaller Genome)
	if (genes.size()<(g->genes).size()) {
		crosspoint=randint(0,(genes.size())-1);
		p1gene=genes.begin();
		p2gene=(g->genes).begin();
		stopper=(g->genes).end();
		p1stop=genes.end();
		p2stop=(g->genes).end();
	}
	else {
		crosspoint=randint(0,((g->genes).size())-1);
		p2gene=genes.begin();
		p1gene=(g->genes).begin();
		stopper=genes.end();
		p1stop=(g->genes.end());
		p2stop=genes.end();
	}

	genecounter=0;  //Ready to count to crosspoint

	skip=false;  //Default to not skip a Gene
	//Note that we skip when we are on the wrong Genome before
	//crossing

	//Now move through the Genes of each parent until both genomes end
	while(p2gene!=stopper) {

		avgene->enable=true;  //Default to true

		if (p1gene==p1stop) {
			chosengene=*p2gene;
			++p2gene;
		}
		else if (p2gene==p2stop) {
			chosengene=*p1gene;
			++p1gene;
		}
		else {
			//Extract current innovation numbers

			//if (p1gene==g->genes.end()) cout<<"WARNING p1"<<std::endl;
			//if (p2gene==g->genes.end()) cout<<"WARNING p2"<<std::endl;

			p1innov=(*p1gene)->innovation_num;
			p2innov=(*p2gene)->innovation_num;

			if (p1innov==p2innov) {

				//Pick the chosengene depending on whether we've crossed yet
				if (genecounter<crosspoint) {
					chosengene=*p1gene;
				}
				else if (genecounter>crosspoint) {
					chosengene=*p2gene;
				}
				//We are at the crosspoint here
				else {

					//Average them into the avgene
					if (randfloat()>0.5) {
                        avgene->lnk->set_trait_id((*p1gene)->lnk->get_trait_id());
                    } else {
                        avgene->lnk->set_trait_id((*p2gene)->lnk->get_trait_id());
                    }

					//WEIGHTS AVERAGED HERE
					(avgene->lnk)->weight=(((*p1gene)->lnk)->weight+((*p2gene)->lnk)->weight)/2.0;


					if (randfloat()>0.5) (avgene->lnk)->in_node=((*p1gene)->lnk)->in_node;
					else (avgene->lnk)->in_node=((*p2gene)->lnk)->in_node;

					if (randfloat()>0.5) (avgene->lnk)->out_node=((*p1gene)->lnk)->out_node;
					else (avgene->lnk)->out_node=((*p2gene)->lnk)->out_node;

					if (randfloat()>0.5) (avgene->lnk)->is_recurrent=((*p1gene)->lnk)->is_recurrent;
					else (avgene->lnk)->is_recurrent=((*p2gene)->lnk)->is_recurrent;

					avgene->innovation_num=(*p1gene)->innovation_num;
					avgene->mutation_num=((*p1gene)->mutation_num+(*p2gene)->mutation_num)/2.0;

					if ((((*p1gene)->enable)==false)||
						(((*p2gene)->enable)==false)) 
						avgene->enable=false;

					chosengene=avgene;
				}

				++p1gene;
				++p2gene;
				++genecounter;
			}
			else if (p1innov<p2innov) {
				if (genecounter<crosspoint) {
					chosengene=*p1gene;
					++p1gene;
					++genecounter;
				}
				else {
					chosengene=*p2gene;
					++p2gene;
				}
			}
			else if (p2innov<p1innov) {
				++p2gene;
				skip=true;  //Special case: we need to skip to the next iteration
				//becase this Gene is before the crosspoint on the wrong Genome
			}
		}

		//Check to see if the chosengene conflicts with an already chosen gene
		//i.e. do they represent the same link    
		curgene2=newgenes.begin();

		while ((curgene2!=newgenes.end())&&
			(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
			(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&((((*curgene2)->lnk)->is_recurrent)== (((chosengene)->lnk)->is_recurrent)) ))&&
			(!((((((*curgene2)->lnk)->in_node)->node_id)==((((chosengene)->lnk)->out_node)->node_id))&&
			(((((*curgene2)->lnk)->out_node)->node_id)==((((chosengene)->lnk)->in_node)->node_id))&&
			(!((((*curgene2)->lnk)->is_recurrent)))&&
			(!((((chosengene)->lnk)->is_recurrent))) )))
		{

			++curgene2;
		}


		if (curgene2!=newgenes.end()) skip=true;  //Link is a duplicate

		if (!skip) {
			//Now add the chosengene to the baby

			//Next check for the nodes, add them if not in the baby Genome already
			inode=(chosengene->lnk)->in_node;
			onode=(chosengene->lnk)->out_node;

			//Check for inode in the newnodes list
			if (inode->node_id<onode->node_id) {
				//cout<<"inode before onode"<<std::endl;
				//Checking for inode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=inode->node_id)) 
					++curnode;

				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					new_inode=new NNode(inode);

					node_insert(newnodes,new_inode);
				}
				else {
					new_inode=(*curnode);
				}

				//Checking for onode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=onode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					new_onode=new NNode(onode);
					node_insert(newnodes,new_onode);

				}
				else {
					new_onode=(*curnode);
				}
			}
			//If the onode has a higher id than the inode we want to add it first
			else {
				//Checking for onode's existence
				curnode=newnodes.begin();
				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=onode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					new_onode=new NNode(onode);
					node_insert(newnodes,new_onode);
				}
				else {
					new_onode=(*curnode);
				}

				//Checking for inode's existence
				curnode=newnodes.begin();

				while((curnode!=newnodes.end())&&
					((*curnode)->node_id!=inode->node_id)) 
					++curnode;
				if (curnode==newnodes.end()) {
					//Here we know the node doesn't exist so we have to add it
					new_inode=new NNode(inode);
					//newnodes.push_back(new_inode);
					node_insert(newnodes,new_inode);
				}
				else {
					new_inode=(*curnode);
				}

			} //End NNode checking section- NNodes are now in new Genome

			//Add the Gene
			newgenes.push_back(new Gene(chosengene,chosengene->lnk->get_trait_id(),new_inode,new_onode));

		}  //End of if (!skip)

		skip=false;

	}


	delete avgene;  //Clean up used object

	//Return the baby Genome
	return (new Genome(genomeid,newtraits,newnodes,newgenes));

}

double Genome::compatibility(Genome *g) {

	//iterators for moving through the two potential parents' Genes
	std::vector<Gene*>::iterator p1gene;
	std::vector<Gene*>::iterator p2gene;  

	//Innovation numbers
	double p1innov;
	double p2innov;

	//Intermediate value
	double mut_diff;

	//Set up the counters
	double num_disjoint=0.0;
	double num_excess=0.0;
	double mut_diff_total=0.0;
	double num_matching=0.0;  //Used to normalize mutation_num differences

	//Now move through the Genes of each potential parent 
	//until both Genomes end
	p1gene=genes.begin();
	p2gene=(g->genes).begin();
	while(!((p1gene==genes.end())&&
		(p2gene==(g->genes).end()))) {

			if (p1gene==genes.end()) {
				++p2gene;
				num_excess+=1.0;
			}
			else if (p2gene==(g->genes).end()) {
				++p1gene;
				num_excess+=1.0;
			}
			else {
				//Extract current innovation numbers
				p1innov=(*p1gene)->innovation_num;
				p2innov=(*p2gene)->innovation_num;

				if (p1innov==p2innov) {
					num_matching+=1.0;
					mut_diff=((*p1gene)->mutation_num)-((*p2gene)->mutation_num);
					if (mut_diff<0.0) mut_diff=0.0-mut_diff;
					//mut_diff+=trait_compare((*p1gene)->lnk->get_trait(),(*p2gene)->lnk->get_trait()); //CONSIDER TRAIT DIFFERENCES
					mut_diff_total+=mut_diff;

					++p1gene;
					++p2gene;
				}
				else if (p1innov<p2innov) {
					++p1gene;
					num_disjoint+=1.0;
				}
				else if (p2innov<p1innov) {
					++p2gene;
					num_disjoint+=1.0;
				}
			}
		} //End while

		//Return the compatibility number using compatibility formula
		//Note that mut_diff_total/num_matching gives the AVERAGE
		//difference between mutation_nums for any two matching Genes
		//in the Genome

		//Normalizing for genome size
		//return (disjoint_coeff*(num_disjoint/max_genome_size)+
		//  excess_coeff*(num_excess/max_genome_size)+
		//  mutdiff_coeff*(mut_diff_total/num_matching));


		//Look at disjointedness and excess in the absolute (ignoring size)

		return (NEAT::disjoint_coeff*(num_disjoint/1.0)+
			NEAT::excess_coeff*(num_excess/1.0)+
			NEAT::mutdiff_coeff*(mut_diff_total/num_matching));
}

double Genome::trait_compare(Trait *t1,Trait *t2) {

	int id1=t1->trait_id;
	int id2=t2->trait_id;
	int count;
	double params_diff=0.0; //Measures parameter difference

	//See if traits represent different fundamental types of connections
	if ((id1==1)&&(id2>=2)) {
		return 0.5;
	}
	else if ((id2==1)&&(id1>=2)) {
		return 0.5;
	}
	//Otherwise, when types are same, compare the actual parameters
	else {
		if (id1>=2) {
			for (count=0;count<=2;count++) {
				params_diff += fabs(t1->params[count]-t2->params[count]);
			}
			return params_diff/4.0;
		}
		else return 0.0; //For type 1, params are not applicable
	}

}

int Genome::extrons() {
	std::vector<Gene*>::iterator curgene;
	int total=0;

	for(curgene=genes.begin();curgene!=genes.end();curgene++) {
		if ((*curgene)->enable) ++total;
	}

	return total;
}

void Genome::randomize_traits() {

	int numtraits = (int)traits.size();

    for(NNode *node: nodes) {
		int trait_id = randint(1,numtraits); //randomize trait
		node->set_trait_id(trait_id);
	}

    for(Gene *gene: genes) {
		int trait_id = randint(1,numtraits); //randomize trait
		gene->lnk->set_trait_id(trait_id);
	}
}

inline Trait *get_trait(const vector<Trait *> &traits, int trait_id) {
    if(trait_id == 0) return nullptr;

    Trait *t = traits[trait_id - 1];
    assert(t->trait_id == trait_id);
    return t;
}

Trait *Genome::get_trait(NNode *node) {
    return ::get_trait(traits, node->get_trait_id());
}

Trait *Genome::get_trait(Link *link) {
    return ::get_trait(traits, link->get_trait_id());
}

bool Genome::link_exists(NNode *in_node, NNode *out_node, bool is_recurrent) {
    for(Gene *g: genes) {
        Link *lnk = g->lnk;
        if( (lnk->in_node == in_node)
            && (lnk->out_node == out_node)
            && (lnk->is_recurrent == is_recurrent) ) {

            return true;
        }
    }

    return false;
}

bool Genome::is_recur(NNode *in_node, NNode *out_node) {
    //These are used to avoid getting stuck in an infinite loop checking
    //for recursion
    //Note that we check for recursion to control the frequency of
    //adding recurrent links rather than to prevent any paricular
    //kind of error
    int thresh=(nodes.size())*(nodes.size());
    int count = 0;

    if( phenotype->is_recur(in_node->analogue, out_node->analogue, count, thresh) ) {
        return true;
    }

    //ADDED: CONSIDER connections out of outputs recurrent
    //todo: this was fixed to use gen_node_label instead of type,
    //      but not clear if this logic is desirable. Shouldn't it
    //      just be checking if the output node is OUTPUT?
/*
    if (((in_node->gen_node_label)==OUTPUT)||
        ((out_node->gen_node_label)==OUTPUT))
        return true;
*/
    return false;
}

void NEAT::print_Genome_tofile(Genome *g,const char *filename) {

	//ofstream oFile(filename,ios::out);

    std::string file = "nero/data/neat/";
    file += filename;
    //strcpyl(file, 100, "nero/data/neat/", filename, 0);
	std::ofstream oFile(file.c_str());
//	oFile.open(file, std::ostream::Write);

	//Make sure	it worked
	//if (!oFile)	{
	//	cerr<<"Can't open "<<filename<<" for output"<<std::endl;
	//	return 0;
	//}
	g->print_to_file(oFile);

	oFile.close();
}

