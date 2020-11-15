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
 * Purpose: Tests the virtual object layer (H5VL)
 *
 *          This is a minimal test to ensure VOL usage (setting a VOL, etc.)
 *          works as expected. Actual VOL functionality is tested using
 *          other mechanisms.
 */


/* Headers needed */
#include "h5test.h"
#include "H5Iprivate.h"         /* IDs                                  */
#define H5T_FRIEND              /* Suppress error about including H5Tpkg    */
#include "H5Tpkg.h"             /* Datatypes                            */
#define H5VL_FRIEND             /* Suppress error about including H5VLpkg    */
#include "H5VLpkg.h"            /* Virtual Object Layer                 */

/* Filename */
const char *FILENAME[] = {"native_vol_test", NULL};

#define NATIVE_VOL_TEST_GROUP_NAME     "test_group"
#define NATIVE_VOL_TEST_DATASET_NAME   "test_dataset"
#define NATIVE_VOL_TEST_ATTRIBUTE_NAME "test_dataset"
#define NATIVE_VOL_TEST_HARD_LINK_NAME "test_hard_link"
#define NATIVE_VOL_TEST_SOFT_LINK_NAME "test_soft_link"
#define NATIVE_VOL_TEST_MOVE_LINK_NAME "test_move_link"
#define NATIVE_VOL_TEST_COPY_LINK_NAME "test_copy_link"
#define NATIVE_VOL_TEST_DATATYPE_NAME  "test_datatype"

#define N_ELEMENTS 10

/* A VOL class struct to verify registering optional operations */
static int reg_opt_curr_op_val;
static herr_t reg_opt_op_optional(void *obj, int opt_type,
    hid_t dxpl_id, void **req, va_list arguments);
static herr_t reg_opt_datatype_get(void *obj, H5VL_datatype_get_t get_type,
    hid_t dxpl_id, void **req, va_list arguments);
#define REG_OPT_VOL_NAME   "reg_opt"
#define REG_OPT_VOL_VALUE ((H5VL_class_value_t)502)
static const H5VL_class_t reg_opt_vol_g = {
    0,                                              /* version      */
    REG_OPT_VOL_VALUE,                              /* value        */
    REG_OPT_VOL_NAME,                               /* name         */
    0,                                              /* capability flags */
    NULL,                                           /* initialize   */
    NULL,                                           /* terminate    */
    {   /* info_cls */
        (size_t)0,                                  /* size    */
        NULL,                                       /* copy    */
        NULL,                                       /* compare */
        NULL,                                       /* free    */
        NULL,                                       /* to_str  */
        NULL,                                       /* from_str */
    },
    {   /* wrap_cls */
        NULL,                                       /* get_object   */
        NULL,                                       /* get_wrap_ctx */
        NULL,                                       /* wrap_object  */
        NULL,                                       /* unwrap_object */
        NULL,                                       /* free_wrap_ctx */
    },
    {   /* attribute_cls */
        NULL,                                       /* create       */
        NULL,                                       /* open         */
        NULL,                                       /* read         */
        NULL,                                       /* write        */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        reg_opt_op_optional,                        /* optional     */
        NULL                                        /* close        */
    },
    {   /* dataset_cls */
        NULL,                                       /* create       */
        NULL,                                       /* open         */
        NULL,                                       /* read         */
        NULL,                                       /* write        */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        reg_opt_op_optional,                        /* optional     */
        NULL                                        /* close        */
    },
    {   /* datatype_cls */
        NULL,                                       /* commit       */
        NULL,                                       /* open         */
        reg_opt_datatype_get,                       /* get          */
        NULL,                                       /* specific     */
        reg_opt_op_optional,                        /* optional     */
        NULL                                        /* close        */
    },
    {   /* file_cls */
        NULL,                                       /* create       */
        NULL,                                       /* open         */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        reg_opt_op_optional,                        /* optional     */
        NULL                                        /* close        */
    },
    {   /* group_cls */
        NULL,                                       /* create       */
        NULL,                                       /* open         */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        reg_opt_op_optional,                        /* optional     */
        NULL                                        /* close        */
    },
    {   /* link_cls */
        NULL,                                       /* create       */
        NULL,                                       /* copy         */
        NULL,                                       /* move         */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        NULL                                        /* optional     */
    },
    {   /* object_cls */
        NULL,                                       /* open         */
        NULL,                                       /* copy         */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        NULL                                        /* optional     */
    },
    {   /* introspect_cls */
        NULL,                                       /* get_conn_cls */
        NULL,                                       /* opt_query    */
    },
    {   /* request_cls */
        NULL,                                       /* wait         */
        NULL,                                       /* notify       */
        NULL,                                       /* cancel       */
        NULL,                                       /* specific     */
        NULL,                                       /* optional     */
        NULL                                        /* free         */
    },
    {   /* blob_cls */
        NULL,                                       /* put          */
        NULL,                                       /* get          */
        NULL,                                       /* specific     */
        NULL                                        /* optional     */
    },
    {   /* token_cls */
        NULL,                                       /* cmp              */
        NULL,                                       /* to_str           */
        NULL                                        /* from_str         */
    },
    NULL                                            /* optional     */
};

#define FAKE_VOL_NAME  "fake"
#define FAKE_VOL_VALUE ((H5VL_class_value_t)501)

/* A VOL class struct that describes a VOL class with no
 * functionality.
 */
static const H5VL_class_t fake_vol_g = {
    0,              /* version      */
    FAKE_VOL_VALUE, /* value        */
    FAKE_VOL_NAME,  /* name         */
    0,              /* capability flags */
    NULL,           /* initialize   */
    NULL,           /* terminate    */
    {
        /* info_cls */
        (size_t)0, /* size    */
        NULL,      /* copy    */
        NULL,      /* compare */
        NULL,      /* free    */
        NULL,      /* to_str  */
        NULL,      /* from_str */
    },
    {
        /* wrap_cls */
        NULL, /* get_object   */
        NULL, /* get_wrap_ctx */
        NULL, /* wrap_object  */
        NULL, /* unwrap_object */
        NULL, /* free_wrap_ctx */
    },
    {
        /* attribute_cls */
        NULL, /* create       */
        NULL, /* open         */
        NULL, /* read         */
        NULL, /* write        */
        NULL, /* get          */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* close        */
    },
    {
        /* dataset_cls */
        NULL, /* create       */
        NULL, /* open         */
        NULL, /* read         */
        NULL, /* write        */
        NULL, /* get          */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* close        */
    },
    {
        /* datatype_cls */
        NULL, /* commit       */
        NULL, /* open         */
        reg_opt_datatype_get, /* get          */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* close        */
    },
    {
        /* file_cls */
        NULL, /* create       */
        NULL, /* open         */
        NULL, /* get          */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* close        */
    },
    {
        /* group_cls */
        NULL, /* create       */
        NULL, /* open         */
        NULL, /* get          */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* close        */
    },
    {
        /* link_cls */
        NULL, /* create       */
        NULL, /* copy         */
        NULL, /* move         */
        NULL, /* get          */
        NULL, /* specific     */
        NULL  /* optional     */
    },
    {
        /* object_cls */
        NULL, /* open         */
        NULL, /* copy         */
        NULL, /* get          */
        NULL, /* specific     */
        NULL  /* optional     */
    },
    {
        /* introspect_cls */
        NULL, /* get_conn_cls */
        NULL, /* opt_query    */
    },
    {
        /* request_cls */
        NULL, /* wait         */
        NULL, /* notify       */
        NULL, /* cancel       */
        NULL, /* specific     */
        NULL, /* optional     */
        NULL  /* free         */
    },
    {
        /* blob_cls */
        NULL, /* put          */
        NULL, /* get          */
        NULL, /* specific     */
        NULL  /* optional     */
    },
    {
        /* token_cls */
        NULL, /* cmp              */
        NULL, /* to_str           */
        NULL  /* from_str         */
    },
    NULL /* optional     */
};

/*-------------------------------------------------------------------------
 * Function:    reg_opt_op_optional
 *
 * Purpose:     Common callback to perform a connector-specific operation
 *              on an object
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 *-------------------------------------------------------------------------
 */
static herr_t
reg_opt_op_optional(void *obj, int opt_type, hid_t H5_ATTR_UNUSED dxpl_id,
    void H5_ATTR_UNUSED **req, va_list arguments)
{
    int *o = (int *)obj;
    int *args;

    /* Check for receiving correct operation value */
    if(opt_type != reg_opt_curr_op_val)
        return -1;

    /* Check that the object is correct */
    if((-1) != *o)
        return -1;

    /* Update the object, with the operation value */
    *o = opt_type;

    /* Check that the argument is correct */
    args = HDva_arg(arguments, void *);
    if(NULL == args)
        return -1;
    if((-1) != *args)
        return -1;

    /* Update the argument return parameter */
    *args = opt_type;

    return 0;
} /* end reg_opt_op_optional() */

/*-------------------------------------------------------------------------
 * Function:    reg_opt_datatype_get
 *
 * Purpose:     Handles the datatype get callback
 *
 * Note:        This is _strictly_ a testing fixture to support the
 *              exercise_reg_opt_oper() testing routine.  It fakes just
 *              enough of the named datatype VOL callback for the
 *              H5VL_register_using_vol_id() call in that test routine to
 *              succeed.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
reg_opt_datatype_get(void H5_ATTR_UNUSED *obj, H5VL_datatype_get_t get_type,
    hid_t H5_ATTR_UNUSED dxpl_id, void H5_ATTR_UNUSED **req, va_list arguments)
{
    herr_t       ret_value = SUCCEED;    /* Return value */

    if(H5VL_DATATYPE_GET_BINARY == get_type) {
        ssize_t *nalloc = HDva_arg(arguments, ssize_t *);
        void *buf = HDva_arg(arguments, void *);
        size_t size = HDva_arg(arguments, size_t);

        if(H5Tencode(H5T_NATIVE_INT, (unsigned char *)buf, &size) < 0)
            ret_value = FAIL;
        else
            *nalloc = (ssize_t) size;
    } /* end if */
    else
        ret_value = FAIL;

    return ret_value;
} /* end reg_opt_datatype_get() */

/*-------------------------------------------------------------------------
 * Function:    test_vol_registration()
 *
 * Purpose:     Tests if we can load, register, and close a simple
 *              VOL connector.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_vol_registration(void)
{
    hid_t  native_id     = H5I_INVALID_HID;
    hid_t  lapl_id       = H5I_INVALID_HID;
    hid_t  vipl_id       = H5I_INVALID_HID;
    herr_t ret           = SUCCEED;
    htri_t is_registered = FAIL;
    hid_t  vol_id        = H5I_INVALID_HID;
    hid_t  vol_id2       = H5I_INVALID_HID;

    TESTING("VOL registration");

    /* The test/fake VOL connector should not be registered at the start of the test */
    if ((is_registered = H5VLis_connector_registered_by_name(FAKE_VOL_NAME)) < 0)
        TEST_ERROR;
    if (is_registered > 0)
        FAIL_PUTS_ERROR("VOL connector is inappropriately registered");
    if ((is_registered = H5VLis_connector_registered_by_value(FAKE_VOL_VALUE)) < 0)
        TEST_ERROR;
    if (is_registered > 0)
        FAIL_PUTS_ERROR("VOL connector is inappropriately registered");

    /* Test registering a connector with an incorrect property list (SHOULD FAIL) */
    if ((lapl_id = H5Pcreate(H5P_LINK_ACCESS)) < 0)
        TEST_ERROR;
    H5E_BEGIN_TRY { vol_id = H5VLregister_connector(&fake_vol_g, lapl_id); }
    H5E_END_TRY;
    if (H5I_INVALID_HID != vol_id)
        FAIL_PUTS_ERROR("should not be able to register a connector with an incorrect property list");
    if (H5Pclose(lapl_id) < 0)
        TEST_ERROR;

    /* Load a VOL interface
     * The vipl_id does nothing without a VOL that needs it, but we do need to
     * test creating a property list of that class and passing it along as a
     * smoke check.
     */
    if ((vipl_id = H5Pcreate(H5P_VOL_INITIALIZE)) < 0)
        TEST_ERROR;
    if ((vol_id = H5VLregister_connector(&fake_vol_g, vipl_id)) < 0)
        TEST_ERROR;
    if (H5Pclose(vipl_id) < 0)
        TEST_ERROR;

    /* The test/fake VOL connector should be registered now */
    if ((is_registered = H5VLis_connector_registered_by_name(FAKE_VOL_NAME)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");
    if ((is_registered = H5VLis_connector_registered_by_value(FAKE_VOL_VALUE)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");

    /* Re-register a VOL connector */
    if ((vol_id2 = H5VLregister_connector(&fake_vol_g, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* The test/fake VOL connector should still be registered now */
    if ((is_registered = H5VLis_connector_registered_by_name(FAKE_VOL_NAME)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");
    if ((is_registered = H5VLis_connector_registered_by_value(FAKE_VOL_VALUE)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");

    /* Unregister the second test/fake VOL ID */
    if (H5VLunregister_connector(vol_id2) < 0)
        TEST_ERROR;

    /* The test/fake VOL connector should still be registered now */
    if ((is_registered = H5VLis_connector_registered_by_name(FAKE_VOL_NAME)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");
    if ((is_registered = H5VLis_connector_registered_by_value(FAKE_VOL_VALUE)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("VOL connector is un-registered");

    /* Unregister the original test/fake VOL ID */
    if (H5VLunregister_connector(vol_id) < 0)
        TEST_ERROR;

    /* Try to unregister the native VOL connector (should fail) */
    if (H5I_INVALID_HID == (native_id = H5VLget_connector_id_by_name(H5VL_NATIVE_NAME)))
        TEST_ERROR;
    H5E_BEGIN_TRY { ret = H5VLunregister_connector(native_id); }
    H5E_END_TRY;
    if (FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to unregister the native VOL connector");

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5VLunregister_connector(vol_id);
        H5Pclose(lapl_id);
        H5Pclose(vipl_id);
    }
    H5E_END_TRY;
    return FAIL;

} /* end test_vol_registration() */

/*-------------------------------------------------------------------------
 * Function:    test_native_vol_init()
 *
 * Purpose:     Tests if the native VOL connector gets initialized.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_native_vol_init(void)
{
    htri_t is_registered;

    TESTING("Native VOL connector initialization");

    /* The native VOL connector should always be registered */
    if ((is_registered = H5VLis_connector_registered_by_name(H5VL_NATIVE_NAME)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("native VOL connector is un-registered");

    if ((is_registered = H5VLis_connector_registered_by_value(H5VL_NATIVE_VALUE)) < 0)
        TEST_ERROR;
    if (0 == is_registered)
        FAIL_PUTS_ERROR("native VOL connector is un-registered");

    PASSED();
    return SUCCEED;

error:
    return FAIL;

} /* end test_native_vol_init() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_file_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL file operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_file_operation(const char *env_h5_drvr)
{
    hid_t fid        = H5I_INVALID_HID;
    hid_t fid_reopen = H5I_INVALID_HID;
    hid_t fapl_id    = H5I_INVALID_HID;
    hid_t fapl_id2   = H5I_INVALID_HID;
    hid_t fcpl_id    = H5I_INVALID_HID;

    char        filename[1024];
    ssize_t     obj_count;
    hid_t       obj_id_list[1];
    hsize_t     file_size;
    unsigned    intent;
    void *      os_file_handle = NULL;
    H5F_info2_t finfo;
    char        name[32];

    TESTING("Basic VOL file operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    /* Set the file close degree to a non-default value, to make the H5Pequal
     *  work out.  This is kinda odd, but the library's current behavior with
     *  a default value is to return the value chosen (H5F_CLOSE_SEMI) instead
     *  of the default value (H5F_CLOSE_DEFAULT) from the property and then
     *  the H5Pequal doesn't detect that the property lists are the same.  Since
     *  this is the documented behavior for file close degree for many years,
     *  I'm not fighting it, just getting the testing to verify that the VOL
     *  connector property is returned correctly.  -QAK, 2018/11/17
     */
    if (H5Pset_fclose_degree(fapl_id, H5F_CLOSE_SEMI) < 0)
        TEST_ERROR;
    if (H5Pset_metadata_read_attempts(fapl_id, 9) < 0)
        TEST_ERROR

    /* H5Fcreate */
    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;

    /* H5Fget_obj_count */
    if ((obj_count = H5Fget_obj_count(fid, H5F_OBJ_FILE)) < 0)
        TEST_ERROR;
    if ((obj_count = H5Fget_obj_count(fid, H5F_OBJ_ALL)) < 0)
        TEST_ERROR;
    if ((obj_count = H5Fget_obj_count((hid_t)H5F_OBJ_ALL, H5F_OBJ_DATASET)) < 0)
        TEST_ERROR;

    /* H5Fget_obj_ids */
    if ((obj_count = H5Fget_obj_ids(fid, H5F_OBJ_ALL, 2, obj_id_list)) < 0)
        TEST_ERROR;
    if ((obj_count = H5Fget_obj_ids((hid_t)H5F_OBJ_ALL, H5F_OBJ_DATASET, 2, obj_id_list)) < 0)
        TEST_ERROR;

    /* Can't compare VFD properties for split / multi / family VFDs */
    if ((hbool_t)(HDstrcmp(env_h5_drvr, "split") && HDstrcmp(env_h5_drvr, "multi") &&
                  HDstrcmp(env_h5_drvr, "family"))) {
        /* H5Fget_access_plist */
        if ((fapl_id2 = H5Fget_access_plist(fid)) < 0)
            TEST_ERROR;
        if (H5Pequal(fapl_id, fapl_id2) != TRUE)
            TEST_ERROR;
        if (H5Pclose(fapl_id2) < 0)
            TEST_ERROR;
    } /* end if */

    /* H5Fget_create_plist */
    if ((fcpl_id = H5Fget_create_plist(fid)) < 0)
        TEST_ERROR;
    if (H5Pclose(fcpl_id) < 0)
        TEST_ERROR;

    /* H5Fget_filesize */
    if (H5Fget_filesize(fid, &file_size) < 0)
        TEST_ERROR;

    /* Can't retrieve VFD handle for split / multi / family VFDs */
    if ((hbool_t)(HDstrcmp(env_h5_drvr, "split") && HDstrcmp(env_h5_drvr, "multi") &&
                  HDstrcmp(env_h5_drvr, "family"))) {
        /* H5Fget_vfd_handle */
        if (H5Fget_vfd_handle(fid, H5P_DEFAULT, &os_file_handle) < 0)
            TEST_ERROR;
    } /* end if */

    /* H5Fget_intent */
    if (H5Fget_intent(fid, &intent) < 0)
        TEST_ERROR;

    /* H5Fget_info2 */
    if (H5Fget_info2(fid, &finfo) < 0)
        TEST_ERROR;

    /* H5Fget_name */
    if (H5Fget_name(fid, name, 32) < 0)
        TEST_ERROR;

    /* H5Fclear_elink_file_cache */
    if (H5Fclear_elink_file_cache(fid) < 0)
        TEST_ERROR;

    /* H5Fflush */
    if (H5Fflush(fid, H5F_SCOPE_GLOBAL) < 0)
        TEST_ERROR;

    /* H5Fclose */
    if (H5Fclose(fid) < 0)
        TEST_ERROR;

    /* H5Fis_accessible */
    if (H5Fis_accessible(filename, fapl_id) < 0)
        TEST_ERROR;

    /* H5Fopen */
    if ((fid = H5Fopen(filename, H5F_ACC_RDWR, fapl_id)) < 0)
        TEST_ERROR;

    /* Can't compare VFD properties for split / multi / family VFDs */
    if ((hbool_t)(HDstrcmp(env_h5_drvr, "split") && HDstrcmp(env_h5_drvr, "multi") &&
                  HDstrcmp(env_h5_drvr, "family"))) {
        /* H5Fget_access_plist */
        if ((fapl_id2 = H5Fget_access_plist(fid)) < 0)
            TEST_ERROR;
        if (H5Pequal(fapl_id, fapl_id2) != TRUE)
            TEST_ERROR;
        if (H5Pclose(fapl_id2) < 0)
            TEST_ERROR;
    } /* end if */

    if ((fid_reopen = H5Freopen(fid)) < 0)
        TEST_ERROR;

    /* Can't compare VFD properties for split / multi / family VFDs */
    if ((hbool_t)(HDstrcmp(env_h5_drvr, "split") && HDstrcmp(env_h5_drvr, "multi") &&
                  HDstrcmp(env_h5_drvr, "family"))) {
        /* H5Fget_access_plist */
        if ((fapl_id2 = H5Fget_access_plist(fid_reopen)) < 0)
            TEST_ERROR;
        if (H5Pequal(fapl_id, fapl_id2) != TRUE)
            TEST_ERROR;
        if (H5Pclose(fapl_id2) < 0)
            TEST_ERROR;
    } /* end if */

    if (H5Fclose(fid) < 0)
        TEST_ERROR;
    if (H5Fclose(fid_reopen) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Fclose(fid_reopen);
        H5Pclose(fapl_id);
        H5Pclose(fapl_id2);
        H5Pclose(fcpl_id);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_file_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_group_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL group operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_group_operation(void)
{
    hid_t      fid     = H5I_INVALID_HID;
    hid_t      fapl_id = H5I_INVALID_HID;
    hid_t      gid     = H5I_INVALID_HID;
    hid_t      gid_a   = H5I_INVALID_HID;
    hid_t      gcpl_id = H5I_INVALID_HID;
    char       filename[1024];
    H5G_info_t info;

    TESTING("Basic VOL group operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;

    /* H5Gcreate */
    if ((gid = H5Gcreate2(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Gget_create_plist */
    if ((gcpl_id = H5Gget_create_plist(gid)) < 0)
        TEST_ERROR;
    if (H5Pclose(gcpl_id) < 0)
        TEST_ERROR;

    /* H5Gget_info */
    if (H5Gget_info(gid, &info) < 0)
        TEST_ERROR;
    if (H5Gget_info(fid, &info) < 0)
        TEST_ERROR;

    /* H5Gget_info_by_name */
    if (H5Gget_info_by_name(fid, NATIVE_VOL_TEST_GROUP_NAME, &info, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Gget_info_by_idx */
    if (H5Gget_info_by_idx(fid, "/", H5_INDEX_NAME, H5_ITER_NATIVE, 0, &info, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Gflush */
    if (H5Gflush(gid) < 0)
        TEST_ERROR;

    /* H5Gclose */
    if (H5Gclose(gid) < 0)
        TEST_ERROR;

    /* H5Gopen */
    if ((gid = H5Gopen2(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Gcreate_anon */
    if ((gid_a = H5Gcreate_anon(fid, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Grefresh */
    if (H5Grefresh(gid) < 0)
        TEST_ERROR;

    if (H5Gclose(gid) < 0)
        TEST_ERROR;
    if (H5Gclose(gid_a) < 0)
        TEST_ERROR;
    if (H5Fclose(fid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Gclose(gid);
        H5Pclose(fapl_id);
        H5Pclose(gcpl_id);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_group_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_dataset_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL dataset operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_dataset_operation(void)
{
    hid_t fid     = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;
    hid_t dcpl_id = H5I_INVALID_HID;
    hid_t dapl_id = H5I_INVALID_HID;
    hid_t did     = H5I_INVALID_HID;
    hid_t did_a   = H5I_INVALID_HID;
    hid_t sid     = H5I_INVALID_HID;
    hid_t tid     = H5I_INVALID_HID;

    char filename[1024];

    hsize_t curr_dims = 0;
    hsize_t max_dims  = H5S_UNLIMITED;

    hsize_t            storage_size;
    haddr_t            offset;
    H5D_space_status_t status;

    int in_buf[N_ELEMENTS];
    int out_buf[N_ELEMENTS];

    int i;

    TESTING("Basic VOL dataset operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;
    for (i = 0; i < N_ELEMENTS; i++) {
        in_buf[i]  = i;
        out_buf[i] = 0;
    }

    /* H5Dcreate */
    curr_dims = 0;
    if ((sid = H5Screate_simple(1, &curr_dims, &max_dims)) < 0)
        TEST_ERROR;
    curr_dims = N_ELEMENTS;
    if ((dcpl_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        TEST_ERROR;
    if (H5Pset_chunk(dcpl_id, 1, &curr_dims) < 0)
        TEST_ERROR;
    if ((did = H5Dcreate2(fid, NATIVE_VOL_TEST_DATASET_NAME, H5T_NATIVE_INT, sid, H5P_DEFAULT, dcpl_id,
                          H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Dcreate_anon */
    if ((did_a = H5Dcreate_anon(fid, H5T_NATIVE_INT, sid, dcpl_id, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    if (H5Sclose(sid) < 0)
        TEST_ERROR;
    if (H5Pclose(dcpl_id) < 0)
        TEST_ERROR;

    /* H5Dset_extent */
    curr_dims = N_ELEMENTS;
    if (H5Dset_extent(did, &curr_dims) < 0)
        TEST_ERROR;

    /* H5Dflush */
    if (H5Dflush(did) < 0)
        TEST_ERROR;

    /* H5Dwrite */
    if (H5Dwrite(did, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, in_buf) < 0)
        TEST_ERROR;

    /* H5Drefresh */
    if (H5Drefresh(did) < 0)
        TEST_ERROR;

    /* H5Dclose */
    if (H5Dclose(did) < 0)
        TEST_ERROR;
    if (H5Dclose(did_a) < 0)
        TEST_ERROR;

    /* H5Dopen */
    if ((did = H5Dopen2(fid, NATIVE_VOL_TEST_DATASET_NAME, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Dget_space */
    if ((sid = H5Dget_space(did)) < 0)
        TEST_ERROR;
    if (H5Sclose(sid) < 0)
        TEST_ERROR;

    /* H5Dget_space_status */
    if (H5Dget_space_status(did, &status) < 0)
        TEST_ERROR;

    /* H5Dget_type */
    if ((tid = H5Dget_type(did)) < 0)
        TEST_ERROR;
    if (H5Tclose(tid) < 0)
        TEST_ERROR;

    /* H5Tcopy (when used w/ a dataset, it gets an H5VL struct */
    if ((tid = H5Tcopy(did)) < 0)
        TEST_ERROR;
    if (H5Tclose(tid) < 0)
        TEST_ERROR;

    /* H5Dget_create_plist */
    if ((dcpl_id = H5Dget_create_plist(did)) < 0)
        TEST_ERROR;
    if (H5Pclose(dcpl_id) < 0)
        TEST_ERROR;

    /* H5Dget_access_plist */
    if ((dapl_id = H5Dget_access_plist(did)) < 0)
        TEST_ERROR;
    if (H5Pclose(dapl_id) < 0)
        TEST_ERROR;

    /* H5Dget_storage_size */
    /* XXX: This is a terrible API call that can't truly indicate failure */
    if (0 == (storage_size = H5Dget_storage_size(did)))
        TEST_ERROR;

    /* H5Dget_offset */
    /* XXX: Another bad API call that can't flag error values. Also, this
     *      returns HADDR_UNDEF for chunked datasets, which is bizarre.
     */
    if (HADDR_UNDEF != (offset = H5Dget_offset(did)))
        TEST_ERROR;

    /* H5Dread */
    if (H5Dread(did, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, out_buf) < 0)
        TEST_ERROR;

    for (i = 0; i < N_ELEMENTS; i++)
        if (in_buf[i] != out_buf[i])
            TEST_ERROR;

    if (H5Dclose(did) < 0)
        TEST_ERROR;
    if (H5Fclose(fid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Dclose(did);
        H5Dclose(did_a);
        H5Sclose(sid);
        H5Tclose(tid);
        H5Pclose(fapl_id);
        H5Pclose(dapl_id);
        H5Pclose(dcpl_id);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_dataset_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_attribute_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL attribute operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_attribute_operation(void)
{
    hid_t fid      = H5I_INVALID_HID;
    hid_t fapl_id  = H5I_INVALID_HID;
    hid_t gid      = H5I_INVALID_HID;
    hid_t aid      = H5I_INVALID_HID;
    hid_t aid_name = H5I_INVALID_HID;
    hid_t sid      = H5I_INVALID_HID;

    char filename[1024];

    hsize_t dims = 1;

    int data_in  = 42;
    int data_out = 0;

    TESTING("Basic VOL attribute operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;
    if ((gid = H5Gcreate2(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;
    dims = 1;
    if ((sid = H5Screate_simple(1, &dims, &dims)) < 0)
        TEST_ERROR;

    /* H5Acreate */
    if ((aid = H5Acreate2(fid, NATIVE_VOL_TEST_ATTRIBUTE_NAME, H5T_NATIVE_INT, sid, H5P_DEFAULT,
                          H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Awrite */
    if (H5Awrite(aid, H5T_NATIVE_INT, &data_in) < 0)
        TEST_ERROR;

    /* H5Aread */
    if (H5Aread(aid, H5T_NATIVE_INT, &data_out) < 0)
        TEST_ERROR;
    if (data_in != data_out)
        TEST_ERROR;

    /* H5Aclose */
    if (H5Aclose(aid) < 0)
        TEST_ERROR;

    /* H5Aopen */
    if ((aid = H5Aopen(fid, NATIVE_VOL_TEST_ATTRIBUTE_NAME, H5P_DEFAULT)) < 0)
        TEST_ERROR;
    if (H5Aclose(aid) < 0)
        TEST_ERROR;

    /* H5Adelete */
    if (H5Adelete(fid, NATIVE_VOL_TEST_ATTRIBUTE_NAME) < 0)
        TEST_ERROR;

    /* H5Acreate_by_name */
    if ((aid_name = H5Acreate_by_name(fid, NATIVE_VOL_TEST_GROUP_NAME, NATIVE_VOL_TEST_ATTRIBUTE_NAME,
                                      H5T_NATIVE_INT, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;
    /* H5Aclose */
    if (H5Aclose(aid_name) < 0)
        TEST_ERROR;

    /* H5Adelete_by_name */
    if (H5Adelete_by_name(fid, NATIVE_VOL_TEST_GROUP_NAME, NATIVE_VOL_TEST_ATTRIBUTE_NAME, H5P_DEFAULT) < 0)
        TEST_ERROR;

    if (H5Sclose(sid) < 0)
        TEST_ERROR;
    if (H5Gclose(gid) < 0)
        TEST_ERROR;
    if (H5Fclose(fid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Pclose(fapl_id);
        H5Gclose(gid);
        H5Sclose(sid);
        H5Aclose(aid);
        H5Aclose(aid_name);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_attribute_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_object_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL object operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_object_operation(void)
{
    hid_t fid     = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;
    hid_t gid     = H5I_INVALID_HID;
    hid_t oid     = H5I_INVALID_HID;

    char        filename[1024];
    H5O_info2_t object_info;

    TESTING("Basic VOL object operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;
    if ((gid = H5Gcreate2(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Oget_info */
    if (H5Oget_info3(fid, &object_info, H5O_INFO_ALL) < 0)
        TEST_ERROR;

    /* H5Oget_info_by_name */
    if (H5Oget_info_by_name3(fid, NATIVE_VOL_TEST_GROUP_NAME, &object_info, H5O_INFO_ALL, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Oexists_by_name */
    if (H5Oexists_by_name(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT) != TRUE)
        TEST_ERROR;

    /* H5Oopen/close */
    if ((oid = H5Oopen(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT)) < 0)
        TEST_ERROR;
    if (H5Oclose(oid) < 0)
        TEST_ERROR;

    if (H5Fclose(fid) < 0)
        TEST_ERROR;
    if (H5Gclose(gid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Pclose(fapl_id);
        H5Gclose(gid);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_object_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_link_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL link operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_link_operation(void)
{
    hid_t fid     = H5I_INVALID_HID;
    hid_t gid     = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;
    char  filename[1024];

    TESTING("Basic VOL link operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;
    if ((gid = H5Gcreate2(fid, NATIVE_VOL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Lcreate_hard */
    if (H5Lcreate_hard(fid, "/", gid, NATIVE_VOL_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Lcreate_soft (to itself) */
    if (H5Lcreate_soft("/", fid, NATIVE_VOL_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Lexists */
    if (H5Lexists(gid, NATIVE_VOL_TEST_HARD_LINK_NAME, H5P_DEFAULT) < 0)
        TEST_ERROR;
    if (H5Lexists(fid, NATIVE_VOL_TEST_SOFT_LINK_NAME, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Lcopy */
    if (H5Lcopy(gid, NATIVE_VOL_TEST_HARD_LINK_NAME, fid, NATIVE_VOL_TEST_COPY_LINK_NAME, H5P_DEFAULT,
                H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Lmove */
    if (H5Lmove(fid, NATIVE_VOL_TEST_COPY_LINK_NAME, gid, NATIVE_VOL_TEST_MOVE_LINK_NAME, H5P_DEFAULT,
                H5P_DEFAULT) < 0)
        TEST_ERROR;

    if (H5Fclose(fid) < 0)
        TEST_ERROR;
    if (H5Gclose(gid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Fclose(fid);
        H5Fclose(gid);
        H5Pclose(fapl_id);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_link_operation() */

/*-------------------------------------------------------------------------
 * Function:    test_basic_datatype_operation()
 *
 * Purpose:     Uses the native VOL connector to test basic VOL datatype operations
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_basic_datatype_operation(void)
{
    hid_t fid      = H5I_INVALID_HID;
    hid_t fapl_id  = H5I_INVALID_HID;
    hid_t tid      = H5I_INVALID_HID;
    hid_t tid_anon = H5I_INVALID_HID;
    hid_t tcpl_id  = H5I_INVALID_HID;
    char  filename[1024];

    TESTING("Basic VOL datatype operations");

    /* Retrieve the file access property for testing */
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if ((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        TEST_ERROR;
    if ((tid = H5Tcopy(H5T_NATIVE_INT)) < 0)
        TEST_ERROR;

    /* H5Tcommit */
    if (H5Tcommit2(fid, NATIVE_VOL_TEST_DATATYPE_NAME, tid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0)
        TEST_ERROR;

    /* H5Tflush */
    if (H5Tflush(tid) < 0)
        TEST_ERROR;

    /* H5Trefresh */
    if (H5Trefresh(tid) < 0)
        TEST_ERROR;

    /* H5Tclose */
    if (H5Tclose(tid) < 0)
        TEST_ERROR;

    /* H5Topen */
    if ((tid = H5Topen2(fid, NATIVE_VOL_TEST_DATATYPE_NAME, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* H5Tget_create_plist */
    if ((tcpl_id = H5Tget_create_plist(tid)) < 0)
        TEST_ERROR;

    /* H5Tcommit_anon */
    if ((tid_anon = H5Tcopy(H5T_NATIVE_INT)) < 0)
        TEST_ERROR;
    if (H5Tcommit_anon(fid, tid_anon, H5P_DEFAULT, H5P_DEFAULT) < 0)
        TEST_ERROR;

    if (H5Pclose(tcpl_id) < 0)
        TEST_ERROR;
    if (H5Tclose(tid) < 0)
        TEST_ERROR;
    if (H5Tclose(tid_anon) < 0)
        TEST_ERROR;
    if (H5Fclose(fid) < 0)
        TEST_ERROR;

    h5_delete_test_file(FILENAME[0], fapl_id);

    /* H5Pclose */
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR;

    PASSED();
    return SUCCEED;

error:
    H5E_BEGIN_TRY
    {
        H5Pclose(tcpl_id);
        H5Fclose(fid);
        H5Pclose(fapl_id);
        H5Tclose(tid);
        H5Tclose(tid_anon);
    }
    H5E_END_TRY;

    return FAIL;

} /* end test_basic_datatype_operation() */

typedef herr_t (*reg_opt_oper_t)(hid_t obj_id, int opt_type, hid_t dxpl_id, void **req, ...);

/*-------------------------------------------------------------------------
 * Function:    exercise_reg_opt_oper()
 *
 * Purpose:     Exercise a particular optional operation for a type.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
exercise_reg_opt_oper(hid_t fake_vol_id, hid_t reg_opt_vol_id,
    H5VL_subclass_t subcls, const char *subcls_name, H5I_type_t id_type,
    reg_opt_oper_t reg_opt_op)
{
    char op_name[256];                  /* Operation name to register */
    hid_t obj_id = H5I_INVALID_HID;
    H5VL_object_t *vol_obj;
    int fake_obj, fake_arg;
    int op_val = -1, op_val2 = -1;
    int find_op_val;
    herr_t ret = SUCCEED;

    /* Test registering optional operation */
    HDsnprintf(op_name, sizeof(op_name), "%s-op1", subcls_name);
    if(H5VLregister_opt_operation(subcls, op_name, &op_val) < 0)
        TEST_ERROR;

    /* Verify that the reserved amount of optional operations is obeyed */
    /* (The first optional operation registered should be at the lower limit) */
    if(op_val != H5VL_RESERVED_NATIVE_OPTIONAL)
        TEST_ERROR;

    /* Look up 1st registered optional operation */
    find_op_val = 0;
    if(H5VLfind_opt_operation(subcls, op_name, &find_op_val) < 0)
        TEST_ERROR;

    /* Verify that the operation was looked up successfully */
    if(op_val != find_op_val)
        TEST_ERROR;

    /* Test registering second optional operation */
    HDsnprintf(op_name, sizeof(op_name), "%s-op2", subcls_name);
    if(H5VLregister_opt_operation(subcls, op_name, &op_val2) < 0)
        TEST_ERROR;

    /* Verify that the reserved amount of optional operations is obeyed */
    /* (The 2nd optional operation registered should be at the lower limit + 1) */
    if(op_val2 != (H5VL_RESERVED_NATIVE_OPTIONAL + 1))
        TEST_ERROR;

    /* Look up 2nd registered optional operation */
    find_op_val = 0;
    if(H5VLfind_opt_operation(subcls, op_name, &find_op_val) < 0)
        TEST_ERROR;

    /* Verify that the operation was looked up successfully */
    if(op_val2 != find_op_val)
        TEST_ERROR;


    /* Push a new API context on the stack */
    /* (Necessary for the named datatype construction routines) */
    if(H5VL_SUBCLS_DATATYPE == subcls)
        H5CX_push();

    /* Create fake object on fake VOL connector */
    if(H5I_INVALID_HID == (obj_id = H5VL_register_using_vol_id(id_type, &fake_obj, fake_vol_id, TRUE)))
        TEST_ERROR;

    /* Pop the API context off the stack */
    if(H5VL_SUBCLS_DATATYPE == subcls)
        H5CX_pop(FALSE);

    /* Attempt to issue operation on fake VOL connector */
    fake_obj = -1;
    fake_arg = -1;
    H5E_BEGIN_TRY {
        ret = (*reg_opt_op)(obj_id, op_val, H5P_DEFAULT, H5_REQUEST_NULL, &fake_arg);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to perform an optional operation with a NULL callback");
    if((-1) != fake_obj)
        FAIL_PUTS_ERROR("'fake_obj' changed during failed operation?");
    if((-1) != fake_arg)
        FAIL_PUTS_ERROR("'fake_arg' changed during failed operation?");

    /* Named datatypes must be destroyed differently */
    if(H5VL_SUBCLS_DATATYPE == subcls) {
        H5T_t *dt;

        /* Destroy fake datatype object */
        if(NULL == (dt = H5I_remove(obj_id)))
            TEST_ERROR;
        if(H5VL_free_object(dt->vol_obj) < 0)
            TEST_ERROR;
        dt->vol_obj = NULL;
        if(H5T_close(dt) < 0)
            TEST_ERROR;
    } /* end if */
    else {
        /* Destroy fake object */
        if(NULL == (vol_obj = H5I_remove(obj_id)))
            TEST_ERROR;
        if(H5VL_free_object(vol_obj) < 0)
            TEST_ERROR;
    } /* end else */


    /* Push a new API context on the stack */
    /* (Necessary for the named datatype construction routines) */
    if(H5VL_SUBCLS_DATATYPE == subcls)
        H5CX_push();

    /* Create fake object on reg_opt VOL connector */
    if(H5I_INVALID_HID == (obj_id = H5VL_register_using_vol_id(id_type, &fake_obj, reg_opt_vol_id, TRUE)))
        TEST_ERROR;

    /* Pop the API context off the stack */
    if(H5VL_SUBCLS_DATATYPE == subcls)
        H5CX_pop(FALSE);

    /* Issue first operation */
    fake_obj = -1;
    fake_arg = -1;
    reg_opt_curr_op_val = op_val;
    if((*reg_opt_op)(obj_id, op_val, H5P_DEFAULT, H5_REQUEST_NULL, &fake_arg) < 0)
        TEST_ERROR;

    /* Verify that fake object & argument were modified correctly */
    if(op_val != fake_obj)
        FAIL_PUTS_ERROR("'fake_obj' not updated");
    if(op_val != fake_arg)
        FAIL_PUTS_ERROR("'fake_arg' not updated");

    /* Issue second operation */
    fake_obj = -1;
    fake_arg = -1;
    reg_opt_curr_op_val = op_val2;
    if((*reg_opt_op)(obj_id, op_val2, H5P_DEFAULT, H5_REQUEST_NULL, &fake_arg) < 0)
        TEST_ERROR;

    /* Verify that fake object & argument were modified correctly */
    if(op_val2 != fake_obj)
        FAIL_PUTS_ERROR("'fake_obj' not updated");
    if(op_val2 != fake_arg)
        FAIL_PUTS_ERROR("'fake_arg' not updated");

    /* Named datatypes must be destroyed differently */
    if(H5VL_SUBCLS_DATATYPE == subcls) {
        H5T_t *dt;

        /* Destroy fake datatype object */
        if(NULL == (dt = H5I_remove(obj_id)))
            TEST_ERROR;
        if(H5VL_free_object(dt->vol_obj) < 0)
            TEST_ERROR;
        dt->vol_obj = NULL;
        if(H5T_close(dt) < 0)
            TEST_ERROR;
    } /* end if */
    else {
        /* Destroy fake object */
        if(NULL == (vol_obj = H5I_remove(obj_id)))
            TEST_ERROR;
        if(H5VL_free_object(vol_obj) < 0)
            TEST_ERROR;
    } /* end else */

    /* Unregister 2nd registered optional operation */
    if(H5VLunregister_opt_operation(subcls, op_name) < 0)
        TEST_ERROR;

    return SUCCEED;

error:
    return FAIL;
} /* end exercise_reg_opt_oper() */

/*-------------------------------------------------------------------------
 * Function:    test_register_opt_operation()
 *
 * Purpose:     Tests if we can load, register, and close a simple
 *              VOL connector.
 *
 * Return:      SUCCEED/FAIL
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_register_opt_operation(void)
{
    hid_t fake_vol_id = H5I_INVALID_HID;
    hid_t reg_opt_vol_id = H5I_INVALID_HID;
    struct {
        H5VL_subclass_t subcls;
        const char *subcls_name;
        H5I_type_t id_type;
        reg_opt_oper_t reg_opt_op;
    } test_params[] = {
        {H5VL_SUBCLS_ATTR, "attr", H5I_ATTR, H5VLattr_optional_op},
        {H5VL_SUBCLS_DATASET, "dataset", H5I_DATASET, H5VLdataset_optional_op},
        {H5VL_SUBCLS_DATATYPE, "datatype", H5I_DATATYPE, H5VLdatatype_optional_op},
        {H5VL_SUBCLS_FILE, "file", H5I_FILE, H5VLfile_optional_op},
        {H5VL_SUBCLS_GROUP, "group", H5I_GROUP, H5VLgroup_optional_op}
    };
    int op_val = -1;
    unsigned u;
    herr_t ret = SUCCEED;

    TESTING("dynamically registering optional operations");

    /* Register the VOL connectors for testing */
    if((fake_vol_id = H5VLregister_connector(&fake_vol_g, H5P_DEFAULT)) < 0)
        TEST_ERROR;
    if((reg_opt_vol_id = H5VLregister_connector(&reg_opt_vol_g, H5P_DEFAULT)) < 0)
        TEST_ERROR;

    /* Test registering invalid optional VOL subclass operations */
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_NONE, "fail", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'NONE' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_INFO, "fail2", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'INFO' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_WRAP, "fail3", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'WRAP' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_LINK, "fail4", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'LINK' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_OBJECT, "fail5", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'OBJECT' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_REQUEST, "fail6", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'REQUEST' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_BLOB, "fail7", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'BLOB' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_TOKEN, "fail8", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation for the 'TOKEN' VOL subclass");
    if((-1) != op_val)
        FAIL_PUTS_ERROR("'op_val' changed during failed operation?");

    /* Test registering valid optional VOL subclass operation with NULL op_val ptr*/
    H5E_BEGIN_TRY {
        ret = H5VLregister_opt_operation(H5VL_SUBCLS_FILE, "fail9", NULL);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to register an optional operation with a NULL 'op_val'");

    /* Try finding a non-existent optional VOL subclass operation */
    H5E_BEGIN_TRY {
        ret = H5VLfind_opt_operation(H5VL_SUBCLS_DATASET, "fail", &op_val);
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to find a non-existent optional operation");

    /* Try unregistering a non-existent optional VOL subclass operation */
    H5E_BEGIN_TRY {
        ret = H5VLunregister_opt_operation(H5VL_SUBCLS_DATASET, "fail");
    } H5E_END_TRY;
    if(FAIL != ret)
        FAIL_PUTS_ERROR("should not be able to unregister a non-existent optional operation");

    /* Register & test calling optional operations for each valid VOL subclass */
    /* (Table-driven, with test_params array) */
    for(u = 0; u < NELMTS(test_params); u++)
        /* Exercise appropriate callback, for each VOL subclass */
        if(exercise_reg_opt_oper(fake_vol_id, reg_opt_vol_id, test_params[u].subcls, test_params[u].subcls_name, test_params[u].id_type, test_params[u].reg_opt_op) < 0)
            TEST_ERROR;

    /* Unregister the VOL connectors */
    if(H5VLunregister_connector(fake_vol_id) < 0)
        TEST_ERROR;
    if(H5VLunregister_connector(reg_opt_vol_id) < 0)
        TEST_ERROR;

    PASSED();

    return SUCCEED;

error:
    H5E_BEGIN_TRY {
        H5VLunregister_connector(fake_vol_id);
        H5VLunregister_connector(reg_opt_vol_id);
    } H5E_END_TRY;

    return FAIL;
} /* end test_register_opt_operation() */

/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests the virtual object layer interface (H5VL)
 *
 * Return:      EXIT_SUCCESS/EXIT_FAILURE
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    const char *env_h5_drvr; /* File driver value from environment */
    int         nerrors = 0;

    /* Get the VFD to use */
    env_h5_drvr = HDgetenv("HDF5_DRIVER");
    if (env_h5_drvr == NULL)
        env_h5_drvr = "nomatch";

    h5_reset();

    HDputs("Testing basic Virtual Object Layer (VOL) functionality.");

    nerrors += test_vol_registration() < 0 ? 1 : 0;
    nerrors += test_register_opt_operation() < 0    ? 1 : 0;
    nerrors += test_native_vol_init() < 0 ? 1 : 0;
    nerrors += test_basic_file_operation(env_h5_drvr) < 0 ? 1 : 0;
    nerrors += test_basic_group_operation() < 0 ? 1 : 0;
    nerrors += test_basic_dataset_operation() < 0 ? 1 : 0;
    nerrors += test_basic_attribute_operation() < 0 ? 1 : 0;
    nerrors += test_basic_object_operation() < 0 ? 1 : 0;
    nerrors += test_basic_link_operation() < 0 ? 1 : 0;
    nerrors += test_basic_datatype_operation() < 0 ? 1 : 0;

    if (nerrors) {
        HDprintf("***** %d Virtual Object Layer TEST%s FAILED! *****\n", nerrors, nerrors > 1 ? "S" : "");
        HDexit(EXIT_FAILURE);
    }

    HDputs("All Virtual Object Layer (VOL) tests passed.");

    HDexit(EXIT_SUCCESS);

} /* end main() */
