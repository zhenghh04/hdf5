/*
 * Copyright (C) 1998 NCSA
 *                    All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Tuesday, March  3, 1998
 *
 * Purpose:	Tests datasets stored in external raw files.
 */
#include <h5test.h>

const char *FILENAME[] = {
    "extern_1",
    "extern_2",
    "extern_3",
    NULL
};


/*-------------------------------------------------------------------------
 * Function:	same_contents
 *
 * Purpose:	Determines whether two files are exactly the same.
 *
 * Return:	Success:	nonzero if same, zero if different.
 *
 *		Failure:	zero
 *
 * Programmer:	Robb Matzke
 *              Wednesday, March  4, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
same_contents (const char *name1, const char *name2)
{
    int		fd1, fd2;
    ssize_t	n1, n2;
    char	buf1[1024], buf2[1024];

    fd1 = open (name1, O_RDONLY);
    fd2 = open (name2, O_RDONLY);
    assert (fd1>=0 && fd2>=0);

    while (1) {
	n1 = read (fd1, buf1, sizeof(buf1));
	n2 = read (fd2, buf2, sizeof(buf2));
	assert (n1>=0 && (size_t)n1<=sizeof(buf1));
	assert (n2>=0 && (size_t)n2<=sizeof(buf2));
	assert (n1==n2);
	
	if (n1<=0 && n2<=0) break;
	if (memcmp (buf1, buf2, (size_t)n1)) {
	    close (fd1);
	    close (fd2);
	    return 0;
	}
    }
    close (fd1);
    close (fd2);
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1a
 *
 * Purpose:	Tests a non-extendible dataset with a single external file.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1a(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*data space current size	*/
    hsize_t	max_size[1];		/*data space maximum size	*/
    int		n;			/*number of external files	*/
    char	name[256];		/*external file name		*/
    off_t	file_offset;		/*external file offset		*/
    hsize_t	file_size;		/*sizeof external file segment	*/
    
    TESTING("fixed-size data space, exact storage");

    /* Create the dataset */
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    cur_size[0] = max_size[0] = 100;
    if (H5Pset_external(dcpl, "ext1.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)))<0) goto error;
    if ((space = H5Screate_simple (1, cur_size, max_size))<0) goto error;
    if ((dset = H5Dcreate (file, "dset1", H5T_NATIVE_INT, space, dcpl))<0)
	goto error;
    if (H5Dclose (dset)<0) goto error;
    if (H5Sclose (space)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;

    /* Read dataset creation information */
    if ((dset = H5Dopen (file, "dset1"))<0) goto error;
    if ((dcpl = H5Dget_create_plist (dset))<0) goto error;
    if ((n=H5Pget_external_count (dcpl))<0) goto error;
    if (1!=n) {
	FAILED();
	puts("    Returned external count is wrong.");
	printf("   got: %d\n    ans: 1\n", n);
	goto error;
    }
    strcpy (name+sizeof(name)-4, "...");
    if (H5Pget_external (dcpl, 0, sizeof(name)-4, name, &file_offset,
			 &file_size)<0) goto error;
    if (file_offset!=0) {
	FAILED();
	puts("    Wrong file offset.");
	printf("    got: %lu\n    ans: 0\n", (unsigned long)file_offset);
	goto error;
    }
    if (file_size!=(max_size[0]*sizeof(int))) {
	FAILED();
	puts("    Wrong file size.");
	printf("    got: %lu\n    ans: %lu\n", (unsigned long)file_size,
		(unsigned long)max_size[0]*sizeof(int));
	goto error;
    }
    if (H5Pclose (dcpl)<0) goto error;
    if (H5Dclose (dset)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Pclose(dcpl);
	H5Sclose(space);
	H5Dclose(dset);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1b
 *
 * Purpose:	Test a single external file which is too small to represent
 *		all the data.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1b(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*current data space size	*/
    hsize_t	max_size[1];		/*maximum data space size	*/

    TESTING("external storage is too small");
    if ((dcpl = H5Pcreate (H5P_DATASET_CREATE))<0) goto error;
    cur_size[0] = max_size[0] = 100;
    if (H5Pset_external(dcpl, "ext1.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)-1))<0) goto error;
    if ((space = H5Screate_simple (1, cur_size, max_size))<0) goto error;
    H5E_BEGIN_TRY {
	dset = H5Dcreate (file, "dset2", H5T_NATIVE_INT, space, dcpl);
    } H5E_END_TRY;
    if (dset>=0) {
	FAILED();
	puts("    Small external file succeeded instead of failing.");
	goto error;
    }
    if (H5Sclose (space)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Sclose(space);
	H5Pclose(dcpl);
	H5Dclose(dset);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1c
 *
 * Purpose:	Test a single external file which is large enough to
 *		represent the current data and large enough to represent the
 *		eventual size of the data.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1c(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*current data space size       */
    hsize_t	max_size[1];		/*maximum data space size	*/

    TESTING("extendible dataspace, exact external size");
    if ((dcpl=H5Pcreate (H5P_DATASET_CREATE))<0) goto error;
    cur_size[0] = 100;
    max_size[0] = 200;
    if (H5Pset_external(dcpl, "ext1.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)))<0) goto error;
    if ((space = H5Screate_simple (1, cur_size, max_size))<0) goto error;
    if ((dset = H5Dcreate (file, "dset3", H5T_NATIVE_INT, space, dcpl))<0)
	goto error;
    if (H5Dclose (dset)<0) goto error;
    if (H5Sclose (space)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(space);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1d
 *
 * Purpose:	Test a single external file which is large enough for the
 *		current data size but not large enough for the eventual size.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1d(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*current data space size       */
    hsize_t	max_size[1];		/*maximum data space size	*/

    TESTING("extendible dataspace, external storage is too small");
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    cur_size[0] = 100;
    max_size[0] = 200;
    if (H5Pset_external(dcpl, "ext1.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)-1))<0) goto error;
    if ((space=H5Screate_simple(1, cur_size, max_size))<0) goto error;
    H5E_BEGIN_TRY {
	dset = H5Dcreate (file, "dset4", H5T_NATIVE_INT, space, dcpl);
    } H5E_END_TRY;
    if (dset>=0) {
	FAILED();
	puts("    Small external file succeeded instead of failing.");
	goto error;
    }
    if (H5Sclose (space)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(space);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1e
 *
 * Purpose:	Test a single external file of unlimited size and an
 *		unlimited data space.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1e(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*data space current size	*/
    hsize_t	max_size[1];		/*data space maximum size	*/
    int		n;			/*number of external files	*/
    char	name[256];		/*external file name		*/
    off_t	file_offset;		/*external file offset		*/
    hsize_t	file_size;		/*sizeof external file segment	*/

    TESTING("unlimited dataspace, unlimited external storage");

    /* Create dataset */
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    if (H5Pset_external(dcpl, "ext1.data", 0, H5F_UNLIMITED)<0) goto error;
    cur_size[0] = 100;
    max_size[0] = H5S_UNLIMITED;
    if ((space=H5Screate_simple(1, cur_size, max_size))<0) goto error;
    if ((dset=H5Dcreate(file, "dset5", H5T_NATIVE_INT, space, dcpl))<0)
	goto error;
    if (H5Dclose (dset)<0) goto error;
    if (H5Sclose (space)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;
    
    /* Read dataset creation information */
    if ((dset = H5Dopen (file, "dset5"))<0) goto error;
    if ((dcpl = H5Dget_create_plist (dset))<0) goto error;
    if ((n = H5Pget_external_count (dcpl))<0) goto error;
    if (1!=n) {
	FAILED();
	puts("    Returned external count is wrong.");
	printf("    got: %d\n    ans: 1\n", n);
	goto error;
    }
    strcpy (name+sizeof(name)-4, "...");
    if (H5Pget_external (dcpl, 0, sizeof(name)-4, name, &file_offset,
			 &file_size)<0) goto error;
    if (file_offset!=0) {
	FAILED();
	puts("    Wrong file offset.");
	printf("    got: %lu\n    ans: 0\n", (unsigned long)file_offset);
	goto error;
    }
    if (H5F_UNLIMITED!=file_size) {
	FAILED();
	puts("    Wrong file size.");
	printf("    got: %lu\n    ans: INF\n", (unsigned long)file_size);
	goto error;
    }
    if (H5Pclose (dcpl)<0) goto error;
    if (H5Dclose (dset)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(space);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1f
 *
 * Purpose:	Test multiple external files for a dataset.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1f(hid_t file)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hsize_t	cur_size[1];		/*data space current size	*/
    hsize_t	max_size[1];		/*data space maximum size	*/

    TESTING("multiple external files");
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    cur_size[0] = max_size[0] = 100;
    if (H5Pset_external(dcpl, "ext1.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)/4))<0) goto error;
    if (H5Pset_external(dcpl, "ext2.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)/4))<0) goto error;
    if (H5Pset_external(dcpl, "ext3.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)/4))<0) goto error;
    if (H5Pset_external(dcpl, "ext4.data", 0,
	(hsize_t)(max_size[0]*sizeof(int)/4))<0) goto error;
    if ((space=H5Screate_simple(1, cur_size, max_size))<0) goto error;
    if ((dset=H5Dcreate(file, "dset6", H5T_NATIVE_INT, space, dcpl))<0)
	goto error;
    if (H5Dclose(dset)<0) goto error;
    if (H5Sclose(space)<0) goto error;
    if (H5Pclose(dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(space);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1g
 *
 * Purpose:	It should be impossible to define an unlimited external file
 *		and then follow it with another external file.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1g(void)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    herr_t	status;			/*function return status	*/
    int		n;			/*number of external files	*/

    TESTING("external file following unlimited file");
    if ((dcpl=H5Pcreate (H5P_DATASET_CREATE))<0) goto error;
    if (H5Pset_external(dcpl, "ext1.data", 0, H5F_UNLIMITED)<0) goto error;
    H5E_BEGIN_TRY {
	status = H5Pset_external(dcpl, "ext2.data", 0, (hsize_t)100);
    } H5E_END_TRY;
    if (status>=0) {
	FAILED();
	puts ("    H5Pset_external() succeeded when it should have failed.");
	goto error;
    }
    if ((n = H5Pget_external_count(dcpl))<0) goto error;
    if (1!=n) {
	FAILED();
	puts("    Wrong external file count returned.");
	goto error;
    }
    if (H5Pclose(dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Pclose(dcpl);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_1h
 *
 * Purpose:	It should be impossible to create a set of external files
 *		whose total size overflows a size_t integer.
 *
 * Return:	Success:	0
 *
 *		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_1h(void)
{
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    herr_t	status;			/*return status			*/
    
    TESTING("address overflow in external files");
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    if (H5Pset_external(dcpl, "ext1.data", 0, H5F_UNLIMITED-1)<0) goto error;
    H5E_BEGIN_TRY {
	status = H5Pset_external(dcpl, "ext2.data", 0, (hsize_t)100);
    } H5E_END_TRY;
    if (status>=0) {
	FAILED();
	puts("    H5Pset_external() succeeded when it should have failed.");
	goto error;
    }
    if (H5Pclose(dcpl)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Pclose(dcpl);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_2
 *
 * Purpose:	Tests reading from an external file set.
 *
 * Return:	Success:	0
 *
 * 		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Wednesday, March  4, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_2 (hid_t fapl)
{
    hid_t	file=-1;		/*file to write to		*/
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	space=-1;		/*data space			*/
    hid_t	dset=-1;		/*dataset			*/
    hid_t	grp=-1;			/*group to emit diagnostics	*/
    int		fd;			/*external file descriptors	*/
    hsize_t	i, j;			/*miscellaneous counters	*/
    hssize_t	n;			/*bytes of I/O			*/
    char	filename[1024];		/*file names			*/
    int		part[25], whole[100];	/*raw data buffers		*/
    hsize_t	cur_size;		/*current data space size	*/
    hid_t	hs_space;		/*hyperslab data space		*/
    hssize_t	hs_start = 30;		/*hyperslab starting offset	*/
    hsize_t	hs_count = 25;		/*hyperslab size		*/
	int temparray[10] = {0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f};

    TESTING("read external dataset");

    /* Write the data to external files directly */
    for (i=0; i<4; i++) {
	for (j=0; j<25; j++) {
	    part[j] = (int)(i*25+j);
	}
	sprintf (filename, "extern_%lua.raw", (unsigned long)i+1);
	fd = HDopen (filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
	assert (fd>=0);
/*	n = lseek (fd, (off_t)(i*10), SEEK_SET);
*/
	n = write(fd,temparray,i*10);
	assert (n>=0 && (size_t)n==i*10);
	n = write (fd, part, sizeof(part));
	assert (n==sizeof(part));
	close (fd);
    }
    
    /*
     * Create the file and an initial group.  This causes messages about
     * debugging to be emitted before we start playing games with what the
     * output looks like.
     */
    h5_fixname(FILENAME[1], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0) {
	goto error;
    }
    if ((grp=H5Gcreate(file, "emit-diagnostics", 8))<0) goto error;
    if (H5Gclose(grp)<0) goto error;

    /* Create the dataset */
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    if (H5Pset_external (dcpl, "extern_1a.raw",  0, (hsize_t)sizeof part)<0 ||
	H5Pset_external (dcpl, "extern_2a.raw", 10, (hsize_t)sizeof part)<0 ||
	H5Pset_external (dcpl, "extern_3a.raw", 20, (hsize_t)sizeof part)<0 ||
	H5Pset_external (dcpl, "extern_4a.raw", 30, (hsize_t)sizeof part)<0)
	goto error;
    cur_size = 100;
    if ((space=H5Screate_simple (1, &cur_size, NULL))<0) goto error;
    if ((dset=H5Dcreate(file, "dset1", H5T_NATIVE_INT, space, dcpl))<0)
	goto error; 

    /*
     * Read the entire dataset and compare with the original
     */
    memset (whole, 0, sizeof(whole));
    if (H5Dread(dset, H5T_NATIVE_INT, space, space, H5P_DEFAULT, whole)<0)
	goto error;
    for (i=0; i<100; i++) {
	if (whole[i]!=(signed)i) {
	    FAILED();
	    puts("    Incorrect value(s) read.");
	    goto error;
	}
    }

    /*
     * Read the middle of the dataset
     */
    if ((hs_space=H5Scopy(space))<0) goto error;
    if (H5Sselect_hyperslab(hs_space, H5S_SELECT_SET, &hs_start, NULL,
			    &hs_count, NULL)<0) goto error;
    memset(whole, 0, sizeof(whole));
    if (H5Dread (dset, H5T_NATIVE_INT, hs_space, hs_space, H5P_DEFAULT,
		 whole)<0) goto error;
    if (H5Sclose (hs_space)<0) goto error;
    for (i=hs_start; i<hs_start+hs_count; i++) {
	if (whole[i]!=(signed)i) {
	    FAILED();
	    puts("    Incorrect value(s) read.");
	    goto error;
	}
    }
    
    if (H5Dclose(dset)<0) goto error;
    if (H5Pclose(dcpl)<0) goto error;
    if (H5Sclose(space)<0) goto error;
    if (H5Fclose(file)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(space);
	H5Fclose(file);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	test_3
 *
 * Purpose:	Tests writing to an external file set.
 *
 * Return:	Success:	0
 *
 * 		Failure:	number of errors
 *
 * Programmer:	Robb Matzke
 *              Wednesday, March  4, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
test_3 (hid_t fapl)
{
    hid_t	file=-1;		/*file to which to write	*/
    hid_t	dcpl=-1;		/*dataset creation properties	*/
    hid_t	mem_space=-1;		/*memory data space		*/
    hid_t	file_space=-1;		/*file data space		*/
    hid_t	dset=-1;		/*dataset			*/
    unsigned	i;			/*miscellaneous counters	*/
    int		fd;			/*external file descriptor	*/
    int		part[25], whole[100];	/*raw data buffers		*/
    hsize_t	cur_size=100;		/*current data space size	*/
    hsize_t	max_size=200;		/*maximum data space size	*/
    hssize_t	hs_start=100;		/*hyperslab starting offset	*/
    hsize_t	hs_count=100;		/*hyperslab size		*/
    char	filename[1024];		/*file name			*/
	int temparray[10] = {0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f};

    TESTING("write external dataset");

    /* Create another file */
    h5_fixname(FILENAME[2], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0) {
	goto error;
    }

    /* Create the external file list */
    if ((dcpl=H5Pcreate(H5P_DATASET_CREATE))<0) goto error;
    if (H5Pset_external(dcpl, "extern_1b.raw", 0, (hsize_t)sizeof part)<0 ||
	H5Pset_external(dcpl, "extern_2b.raw", 10, (hsize_t)sizeof part)<0 ||
	H5Pset_external(dcpl, "extern_3b.raw", 20, (hsize_t)sizeof part)<0 ||
	H5Pset_external(dcpl, "extern_4b.raw", 30, H5F_UNLIMITED)<0)
	goto error;

    /* Make sure the output files are fresh*/
    for (i=1; i<=4; i++) {
	sprintf(filename, "extern_%db.raw", i);
	if ((fd= open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666))<0) {
	    FAILED();
	    printf("    cannot open %s: %s\n", filename, strerror(errno));
	    goto error;
	}
	
	write(fd, temparray, (i-1)*10);
	close (fd);
    }

    /* Create the dataset */
    if ((mem_space=H5Screate_simple(1, &cur_size, &max_size))<0) goto error;
    if ((file_space=H5Scopy(mem_space))<0) goto error;
    if ((dset=H5Dcreate(file, "dset1", H5T_NATIVE_INT, file_space, dcpl))<0)
	goto error;

    /* Write the entire dataset and compare with the original */
    for (i=0; i<cur_size; i++) whole[i] = i;
    if (H5Dwrite(dset, H5T_NATIVE_INT, mem_space, file_space, H5P_DEFAULT,
		 whole)<0) goto error;
    for (i=0; i<4; i++) {
	char name1[64], name2[64];
	sprintf (name1, "extern_%da.raw", i+1);
	sprintf (name2, "extern_%db.raw", i+1);
	if (!same_contents (name1, name2)) {
	    FAILED();
	    puts ("   Output differs from expected value.");
	    goto error;
	}
    }

    /* Extend the dataset by another 100 elements */
    if (H5Dextend(dset, &max_size)<0) goto error;
    if (H5Sclose(file_space)<0) goto error;
    if ((file_space=H5Dget_space(dset))<0) goto error;

    /* Write second half of dataset */
    for (i=0; i<hs_count; i++) whole[i] = 100+i;
    if (H5Sselect_hyperslab(file_space, H5S_SELECT_SET, &hs_start, NULL,
			    &hs_count, NULL)<0) goto error;
    if (H5Dwrite(dset, H5T_NATIVE_INT, mem_space, file_space, H5P_DEFAULT,
		 whole)<0) goto error;


    if (H5Dclose (dset)<0) goto error;
    if (H5Pclose (dcpl)<0) goto error;
    if (H5Sclose (mem_space)<0) goto error;
    if (H5Sclose (file_space)<0) goto error;
    if (H5Fclose (file)<0) goto error;
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Dclose(dset);
	H5Pclose(dcpl);
	H5Sclose(mem_space);
	H5Sclose(file_space);
	H5Fclose(file);
    } H5E_END_TRY;
    return 1;
}


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Runs external dataset tests.
 *
 * Return:	Success:	exit(0)
 *
 *		Failure:	exit(non-zero)
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March  3, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main (void)
{
    hid_t	fapl=-1;		/*file access properties	*/
    hid_t	file=-1;		/*file for test_1* functions	*/
    char	filename[1024];		/*file name for test_1* funcs	*/
    hid_t	grp=-1;			/*group to emit diagnostics	*/
    int		nerrors=0;		/*number of errors		*/
    
    h5_reset();
    fapl = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0) {
	goto error;
    }
    if ((grp=H5Gcreate(file, "emit-diagnostics", 8))<0) goto error;
    if (H5Gclose (grp)<0) goto error;

    nerrors += test_1a(file);
    nerrors += test_1b(file);
    nerrors += test_1c(file);
    nerrors += test_1d(file);
    nerrors += test_1e(file);
    nerrors += test_1f(file);
    nerrors += test_1g();
    nerrors += test_1h();
    nerrors += test_2(fapl);
    nerrors += test_3(fapl);
    if (nerrors>0) goto error;

    if (H5Fclose(file)<0) goto error;
    puts("All external storage tests passed.");
    if (h5_cleanup(FILENAME, fapl)) {
	remove("extern_1a.raw");
	remove("extern_1b.raw");
	remove("extern_2a.raw");
	remove("extern_2b.raw");
	remove("extern_3a.raw");
	remove("extern_3b.raw");
	remove("extern_4a.raw");
	remove("extern_4b.raw");
    }
    return 0;

 error:
    H5E_BEGIN_TRY {
	H5Fclose(file);
	H5Pclose(fapl);
    } H5E_END_TRY;
    nerrors = MAX(1, nerrors);
    printf ("%d TEST%s FAILED.\n", nerrors, 1==nerrors?"":"s");
    return 1;
}
