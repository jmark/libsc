/*
  This file is part of the SC Library.
  The SC Library provides support for parallel scientific applications.

  Copyright (C) 2015 The University of Texas System

  The SC Library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  The SC Library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with the SC Library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include <sc.h>
#include <sc_mpi.h>
#include <sc_options.h>
#include <sc_allgather.h>

int
main (int argc, char **argv)
{
  sc_options_t *opt;
  sc_MPI_Comm   intranode = sc_MPI_COMM_NULL;
  sc_MPI_Comm   internode = sc_MPI_COMM_NULL;
  int           mpiret, node_size = 1, rank, size;
  int           first, intrarank, interrank;

  mpiret = sc_MPI_Init (&argc, &argv);
  SC_CHECK_MPI (mpiret);
  mpiret = sc_MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  SC_CHECK_MPI (mpiret);
  mpiret = sc_MPI_Comm_size (MPI_COMM_WORLD, &size);
  SC_CHECK_MPI (mpiret);

  sc_init (MPI_COMM_WORLD, 1, 1, NULL, SC_LP_DEFAULT);

  opt = sc_options_new (argv[0]);
  sc_options_add_int (opt, 'n', "node-size", &node_size,
                      node_size, "simulated node size for intranode communicators");

  first = sc_options_parse (sc_package_id, SC_LP_INFO, opt, argc, argv);
  if (first < 0) {
    sc_options_print_usage (sc_package_id, SC_LP_INFO, opt, NULL);
    sc_abort_collective ("Usage error");
  }
  sc_options_destroy (opt);

  sc_mpi_comm_attach_node_comms (MPI_COMM_WORLD,node_size);
  sc_mpi_comm_get_node_comms(MPI_COMM_WORLD,&intranode,&internode);

  SC_CHECK_ABORT(intranode != sc_MPI_COMM_NULL,"Could not extract communicator");
  SC_CHECK_ABORT(internode != sc_MPI_COMM_NULL,"Could not extract communicator");

  mpiret = sc_MPI_Comm_rank(intranode,&intrarank);
  SC_CHECK_MPI (mpiret);
  mpiret = sc_MPI_Comm_rank(internode,&interrank);
  SC_CHECK_MPI (mpiret);

  SC_CHECK_ABORT (interrank * node_size + intrarank == rank, "rank calculation mismatch");

  srandom(rank);
  {
    long int myval = random();
    int longintsize = sizeof (long int);
    long int *recv_self;
    long int *scan_self;
    long int *recv_final;
    long int *scan_final;
    int check, p;

    recv_self = SC_ALLOC(long int,size);
    scan_self = SC_ALLOC(long int,size + 1);
    mpiret = sc_MPI_Allgather(&myval,longintsize,sc_MPI_CHAR,
                              recv_self,longintsize,sc_MPI_CHAR, MPI_COMM_WORLD);
    SC_CHECK_MPI(mpiret);

    scan_self[0] = 0;
    for (p = 0; p < size; p++) {
      scan_self[p + 1] = scan_self[p] + recv_self[p];
    }

    sc_allgather_final_create_default (&myval,longintsize,sc_MPI_CHAR,
                               (void *) &recv_final,longintsize,sc_MPI_CHAR, MPI_COMM_WORLD);
    check = memcmp(recv_self,recv_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_create_default does not reproduce sc_MPI_Allgather");
    sc_allgather_final_destroy_default (recv_final,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_default (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_default does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_prescan (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_prescan does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

#if defined(__bgq__)
    sc_allgather_final_create_shared (&myval,longintsize,sc_MPI_CHAR,
                               (void *) &recv_final,longintsize,sc_MPI_CHAR, MPI_COMM_WORLD);
    check = memcmp(recv_self,recv_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_create_shared does not reproduce sc_MPI_Allgather");
    sc_allgather_final_destroy_shared (recv_final,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_shared (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_shared does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_shared_prescan (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_shared_prescan does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);
#endif

    sc_allgather_final_create_window (&myval,longintsize,sc_MPI_CHAR,
                               (void *) &recv_final,longintsize,sc_MPI_CHAR, MPI_COMM_WORLD);
    check = memcmp(recv_self,recv_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_create_window does not reproduce sc_MPI_Allgather");
    sc_allgather_final_destroy_window (recv_final,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_window (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_window does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

    sc_allgather_final_scan_create_window_prescan (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create_window_prescan does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

    sc_allgather_final_create (&myval,longintsize,sc_MPI_CHAR,
                               (void *) &recv_final,longintsize,sc_MPI_CHAR, MPI_COMM_WORLD);
    check = memcmp(recv_self,recv_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_create does not reproduce sc_MPI_Allgather");
    sc_allgather_final_destroy(recv_final,MPI_COMM_WORLD);

    sc_allgather_final_scan_create (&myval,(void *) &scan_final, 1, sc_MPI_LONG, sc_MPI_SUM,
                                            MPI_COMM_WORLD);
    check = memcmp(scan_self,scan_final,longintsize*size);
    SC_CHECK_ABORT(!check,"sc_allgather_final_scan_create does not reproduce sc_MPI_Allgather + scan");
    sc_allgather_final_destroy_default (scan_self,MPI_COMM_WORLD);

    SC_FREE (recv_self);
    SC_FREE (scan_self);
  }

  sc_finalize ();

  mpiret = sc_MPI_Finalize ();
  SC_CHECK_MPI (mpiret);
  return 0;
}
