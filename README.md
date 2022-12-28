# pyobject-preservation
Preserving and resurrecting PyObjects in CPython

# Build

To build this project, you must have
[Miniconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html)
installed.

Then run the following:

```shell
conda env create -f environment.yaml -n pyobject-preservation
```
```shell
conda activate pyobject-preservation
```
```shell
python setup.py install
```

# Run

Run the example script:

```shell
python preservation.py
```
