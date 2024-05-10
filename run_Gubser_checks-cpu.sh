#!/bin/bash

mkdir -p $1

sbatch <<EOT
#!/bin/bash
#SBATCH -A bbkr-delta-cpu
#SBATCH -p cpu
#SBATCH -t 00:20:00
#SBATCH --cpus-per-task=64
#SBATCH --ntasks=1
#SBATCH --output="$1/job.out"
##SBATCH --mem-per-cpu=10250

INPUT_PARAMETERS_FILE=input/input_parameters_gubser_checks.yaml

RESULTS_DIRECTORY=$1
mkdir -p \${RESULTS_DIRECTORY}

# save state of current git branch used to generate these results
GIT_SNAPSHOT_FILE=\${RESULTS_DIRECTORY}/git_snapshot.txt
echo "SHA:" `git rev-parse HEAD` > \${GIT_SNAPSHOT_FILE}
echo "BRANCH:" `git rev-parse --abbrev-ref HEAD` >> \${GIT_SNAPSHOT_FILE}
echo "FULL:" >> \${GIT_SNAPSHOT_FILE}
git log -1 >> \${GIT_SNAPSHOT_FILE}

# save settings file for reference
cp \${INPUT_PARAMETERS_FILE} \${RESULTS_DIRECTORY}

cd apptainer
time -p srun -n 1 -c 64 make run-gubser
cd ..
mv build/output_gubser/* \${RESULTS_DIRECTORY}

exit 0
EOT
