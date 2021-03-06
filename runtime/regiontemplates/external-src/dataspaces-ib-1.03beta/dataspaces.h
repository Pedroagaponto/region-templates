/*
 * Copyright (c) 2009, NSF Cloud and Autonomic Computing Center, Rutgers University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this list of conditions and
 * the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided with the distribution.
 * - Neither the name of the NSF Cloud and Autonomic Computing Center, Rutgers University, nor the names of its
 * contributors may be used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
*  Ciprian Docan (2009)  TASSL Rutgers University
*  docan@cac.rutgers.edu
*  Tong Jin (2011) TASSL Rutgers University
*  tjin@cac.rutgers.edu
*/

#ifndef __DATASPACES_H_
#define __DATASPACES_H_

int dspaces_init(int num_peers, int appid);
void dspaces_set_storage_type (int fst);
int dspaces_rank(void);
int dspaces_peers(void);
void dspaces_barrier(void);
void dspaces_lock_on_read(const char *lock_name);
void dspaces_unlock_on_read(const char *lock_name);
void dspaces_lock_on_write(const char *lock_name);
void dspaces_unlock_on_write(const char *lock_name);
int dspaces_get (const char *var_name, 
        unsigned int ver, int size,
        int xl, int yl, int zl, 
        int xu, int yu, int zu, 
        void *data);
int dspaces_get_versions(int **);
int dspaces_put (const char *var_name, 
        unsigned int ver, int size,
        int xl, int yl, int zl,
        int xu, int yu, int zu, 
        void *data);
int dspaces_select(char *var_name, 
	unsigned int ver, int size,
        int xl, int yl, int zl,
        int xu, int yu, int zu, 
        void *data);
int dspaces_cq_register(char *var_name,
        int xl, int yl, int zl,
        int xu, int yu, int zu, 
        void *data);
int dspaces_cq_update (void);
int dspaces_put_sync(void);
void dspaces_finalize (void);

/* CCGrid'11 demo */
int dspaces_collect_timing(double, double *);
int dspaces_num_space_srv(void);

#endif
