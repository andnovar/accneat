import matplotlib.pyplot as plt
import numpy as np
plt.style.use('ggplot')

runs = 3

def movingaverage (values, window):
    weights = np.repeat(1.0, window)/window
    sma = np.convolve(values, weights, 'valid')
    return sma

i = 1

max_fitnesses = [[], [], []]
nnodes = [[], [], []]
num_species = [[0], [0], [0]]
num_genes = [[], [], []]
gens = [0, 0, 0]

with open("results.txt", 'r') as f:
    for line in f:
        exp = line.find("Experiment ")
        if exp != -1:
            ind_exp = int(line[exp + len("Experiment"):].split('/')[0]) - 1
            gens[ind_exp] += 1
        
        n_species = line.find("Number of Species: ")
        if n_species != -1:
            num_species[ind_exp].append(int(line[n_species + len("Number of Species: "):].rstrip()))
        
        fittest = line.find("fittest")
        if fittest != -1:
            info = line[fittest:].split(',')
            max_fitnesses[ind_exp].append(float(info[0].split('=')[-1]))
            nnodes[ind_exp].append(int(info[-2].split('=')[-1]))
            num_genes[ind_exp].append(int(info[-1].split('=')[-1]))

 
for i in xrange(0, runs):
    generations = list(xrange(gens[i]))
    plt.clf()
    yMA = movingaverage(max_fitnesses[i], 3)
    plt.plot(generations[len(generations)-len(yMA):],yMA, label='Moving Averages')
    plt.plot(generations, max_fitnesses[i], label='Max fitness')
    plt.xlabel('Generations')
    plt.ylabel('Max Fitness over generations')
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=0.)
    plt.savefig('max_fitnesses_over_generations_run_'+str(i))
     
    plt.clf()
    yMA = movingaverage(num_species[i], 3)
    plt.plot(generations[len(generations)-len(yMA):],yMA, label='Moving Averages')
    plt.plot(generations, num_species[i], label='Number of species')
    plt.xlabel('Generations')
    plt.ylabel('Number of species over generations')
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=0.)
    plt.savefig('number_of_species_over_generations_run_'+str(i))
     
    plt.clf()
    yMA = movingaverage(num_genes[i], 3)
    plt.plot(generations[len(generations)-len(yMA):],yMA, label='Moving Averages')
    plt.plot(generations, num_genes[i], label='Number of genes')
    plt.xlabel('Generations')
    plt.ylabel('Number of genes from best individual over generations')
    plt.legend(bbox_to_anchor=(0.05, 1), loc=2, borderaxespad=0.)
    plt.savefig('number_of_genes_over_generations_run_'+str(i))