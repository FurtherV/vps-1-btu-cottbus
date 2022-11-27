#include <mpi.h>

int main(int argc, char **argv) {
    MPI::Init(argc, argv);
    MPI::Finalize();
}
