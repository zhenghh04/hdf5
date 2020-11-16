/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Programmer:  Quincey Koziol <koziol@lbl.gov>
 *              Wednesday, April 8, 2020
 *
 * Purpose:    Tests event sets.
 */
#include "h5test.h"
#include "H5srcdir.h"
#include "nb_vol_conn.h"

const char *FILENAME[] = {"event_set_1", NULL};

/*-------------------------------------------------------------------------
 * Function:    test_es_create
 *
 * Purpose:     Tests creating event sets.
 *
 * Return:      Success:    0
 *              Failure:    number of errors
 *
 * Programmer:  Quincey Koziol
 *              Thursday, April 9, 2020
 *
 *-------------------------------------------------------------------------
 */
static int
test_es_create(void)
{
    hid_t   es_id;        /* Event set ID */
    size_t  count;        /* # of events in set */
    size_t  num_errs;     /* # of failed events in set */
    uint64_t num_ops;     /* # of events inserted into set */
    hbool_t err_occurred; /* Whether an error has occurred */

    TESTING("event set creation");

    /* Create an event set */
    if ((es_id = H5EScreate()) < 0)
        TEST_ERROR;

    /* Query the # of events in empty event set */
    count = 0;
    if (H5ESget_count(es_id, &count) < 0)
        TEST_ERROR;
    if (count > 0)
        FAIL_PUTS_ERROR("should be empty event set");

    /* Check for errors */
    err_occurred = FALSE;
    if (H5ESget_err_status(es_id, &err_occurred) < 0)
        TEST_ERROR;
    if (err_occurred)
        FAIL_PUTS_ERROR("should not be an error for the event set");

    /* Check errors count */
    num_errs = 0;
    if (H5ESget_err_count(es_id, &num_errs) < 0)
        TEST_ERROR;
    if (num_errs)
        FAIL_PUTS_ERROR("should not be any errors for the event set");

    /* Check errors count */
    num_ops = 0;
    if (H5ESget_op_counter(es_id, &num_ops) < 0)
        TEST_ERROR;
    if (num_ops)
        FAIL_PUTS_ERROR("should not be any operations for the event set yet");

    /* Close the event set */
    if (H5ESclose(es_id) < 0)
        TEST_ERROR;

    PASSED();
    return 0;

error:
    H5E_BEGIN_TRY { H5ESclose(es_id); }
    H5E_END_TRY;
    return 1;
}

/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests event sets
 *
 * Return:      Success: EXIT_SUCCESS
 *              Failure: EXIT_FAILURE
 *
 * Programmer:  Quincey Koziol
 *              Wednesday, April 8, 2020
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    H5VL_nonblock_info_t nb_info;                   /* Non-blocking VOL connector info */
    hid_t                fapl_id = H5I_INVALID_HID; /* File access property list */
    int                  nerrors = 0;               /* Error count */

    /* Setup */
    h5_reset();
    fapl_id = h5_fileaccess();

    /* Set up the non-blocking VOL connector's info */
    nb_info.under_vol_id   = H5VL_NATIVE;
    nb_info.under_vol_info = NULL;

    /* Use the non-blocking VOL connector for these tests */
    if (H5Pset_vol(fapl_id, H5VL_NONBLOCK, &nb_info) < 0)
        nerrors++;

    /* Tests */
    nerrors += test_es_create();
#ifdef NOT_YET
    nerrors += test_file_create(fapl_id, FILENAME[0]);
#endif /* NOT_YET */

    /* Cleanup */
    h5_cleanup(FILENAME, fapl_id);

    /* Check for any errors */
    if (nerrors) {
        HDputs("***** EVENT SET TESTS FAILED *****");
        HDexit(EXIT_FAILURE);
    } /* end if */

    /* Report status */
    HDputs("All event set tests passed.");

    HDexit(EXIT_SUCCESS);
} /* end main() */
