/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:	The public header file for the native VOL driver.
 */

#ifndef H5VLnative_H
#define H5VLnative_H

/* Initializer function for native VOL driver */
#define H5VL_NATIVE             (H5VL_native_init())

/* Characteristics of the native VOL driver */
#define H5VL_NATIVE_NAME        "native"
#define H5VL_NATIVE_VALUE       0
#define H5VL_NATIVE_VERSION     0


#ifdef __cplusplus
extern "C" {
#endif

H5_DLL hid_t H5VL_native_init(void);
H5_DLL herr_t H5Pset_fapl_native(hid_t fapl_id);

#ifdef __cplusplus
}
#endif

#endif
