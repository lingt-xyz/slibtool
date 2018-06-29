/*******************************************************************/
/*  slibtool: a skinny libtool implementation, written in C        */
/*  Copyright (C) 2016--2018  Z. Gilboa                            */
/*  Released under the Standard MIT License; see COPYING.SLIBTOOL. */
/*******************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <slibtool/slibtool.h>
#include "slibtool_dprintf_impl.h"
#include "slibtool_errinfo_impl.h"
#include "slibtool_metafile_impl.h"

static int  slbt_create_default_library_wrapper(
	const struct slbt_driver_ctx *	dctx,
	struct slbt_exec_ctx *		ectx,
	char *				arname,
	char *				soname,
	char *				soxyz,
	char *				solnk)
{
	int					ret;
	int					fdout;
	const char *				header;
	const char *				base;
	bool					fnover;
	bool					fvernum;
	int					current;
	int					revision;
	int					age;
	const struct slbt_source_version *	verinfo;

	(void)ectx;

	/* create */
	if ((fdout = openat(
			AT_FDCWD,
			dctx->cctx->output,
			O_RDWR|O_CREAT|O_TRUNC,
			0644)) < 0)
		return SLBT_SYSTEM_ERROR(dctx);

	/* version info */
	current  = 0;
	age      = 0;
	revision = 0;

	if (dctx->cctx->verinfo.verinfo)
		sscanf(dctx->cctx->verinfo.verinfo,"%d:%d:%d",
			&current,&revision,&age);

	fnover  = !!(dctx->cctx->drvflags & SLBT_DRIVER_AVOID_VERSION);
	fvernum = !!(dctx->cctx->verinfo.vernumber);
	verinfo = slbt_source_version();

	/* wrapper header */
	header = "libtool compatible library wrapper\n";
	base   = "";

	/* wrapper content */
	ret = slbt_dprintf(fdout,
		"# %s%s"
		"# Generated by %s (slibtool %d.%d.%d)\n"
		"# [commit reference: %s]\n\n"

		"dlname='%s'\n"
		"library_names='%s %s %s'\n"
		"old_library='%s'\n\n"

		"inherited_linker_flags='%s'\n"
		"dependency_libs='%s'\n"
		"weak_library_names='%s'\n\n"

		"current=%d\n"
		"age=%d\n"
		"revision=%d\n\n"

		"installed=%s\n"
		"shouldnotlink=%s\n\n"

		"dlopen='%s'\n"
		"dlpreopen='%s'\n\n"

		"libdir='%s'\n",

		/* wrapper header */
		base,header,

		/* nickname, verinfo */
		dctx->program,
		verinfo->major,verinfo->minor,verinfo->revision,
		verinfo->commit,

		/* dlname */
		fnover ? solnk : soxyz,

		/* library_names */
		fnover ? solnk : soxyz,
		fnover ? solnk : soname,
		solnk,

		/* old_library */
		arname,

		/* inherited_linker_flags, dependency_libs, weak_library_names */
		"","","",

		/* current, age, revision */
		fvernum ? dctx->cctx->verinfo.major : current,
		fvernum ? dctx->cctx->verinfo.minor : age,
		fvernum ? dctx->cctx->verinfo.major : revision,

		/* installed, shouldnotlink */
		"no","no",

		/* dlopen, dlpreopen */
		"","",

		/* libdir */
		dctx->cctx->rpath ? dctx->cctx->rpath : "");

	close(fdout);

	return (ret < 0) ? SLBT_SYSTEM_ERROR(dctx) : 0;
}

static int  slbt_create_compatible_library_wrapper(
	const struct slbt_driver_ctx *	dctx,
	struct slbt_exec_ctx *		ectx,
	char *				arname,
	char *				soname,
	char *				soxyz,
	char *				solnk)
{
	int					ret;
	FILE *					fout;
	const char *				base;
	bool					fnover;
	bool					fvernum;
	int					current;
	int					revision;
	int					age;
	const struct slbt_source_version *	verinfo;

	(void)ectx;

	/* create */
	if (!(fout = fopen(dctx->cctx->output,"w")))
		return SLBT_SYSTEM_ERROR(dctx);

	/* version info */
	current  = 0;
	age      = 0;
	revision = 0;

	/* wrapper header */
	if ((base = strrchr(dctx->cctx->output,'/'))) {
		base++;
	} else {
		base = dctx->cctx->output;
	}

	if (dctx->cctx->verinfo.verinfo)
		sscanf(dctx->cctx->verinfo.verinfo,"%d:%d:%d",
			&current,&revision,&age);

	fnover  = !!(dctx->cctx->drvflags & SLBT_DRIVER_AVOID_VERSION);
	fvernum = !!(dctx->cctx->verinfo.vernumber);
	verinfo = slbt_source_version();

	/* wrapper content */
	ret = fprintf(fout,
		"# %s - a libtool library file\n"
		"# Generated by %s (slibtool %d.%d.%d)\n"
		"# [commit reference: %s]\n"
		"#\n"
		"# Please DO NOT delete this file!\n"
		"# It is necessary for linking the library.\n\n"

		"# The name that we can dlopen(3).\n"
		"dlname='%s'\n\n"

		"# Names of this library.\n"
		"library_names='%s %s %s'\n\n"

		"# The name of the static archive.\n"
		"old_library='%s'\n\n"

		"# Linker flags that can not go in dependency_libs.\n"
		"inherited_linker_flags='%s'\n\n"

		"# Libraries that this one depends upon.\n"
		"dependency_libs='%s'\n\n"

		"# Names of additional weak libraries provided by this library\n"
		"weak_library_names='%s'\n\n"

		"# Version information for %s%s.\n"
		"current=%d\n"
		"age=%d\n"
		"revision=%d\n\n"

		"# Is this an already installed library?\n"
		"installed=%s\n\n"

		"# Should we warn about portability when linking against -modules?\n"
		"shouldnotlink=%s\n\n"

		"# Files to dlopen/dlpreopen\n"
		"dlopen='%s'\n"
		"dlpreopen='%s'\n\n"

		"# Directory that this library needs to be installed in:\n"
		"libdir='%s'\n",

		/* wrapper header */
		base,

		/* nickname, verinfo */
		dctx->program,
		verinfo->major,verinfo->minor,verinfo->revision,
		verinfo->commit,

		/* dlname */
		fnover ? solnk : soxyz,

		/* library_names */
		fnover ? solnk : soxyz,
		fnover ? solnk : soname,
		solnk,

		/* old_library */
		arname,

		/* inherited_linker_flags, dependency_libs, weak_library_names */
		"","","",

		/* version information for */
		(dctx->cctx->drvflags & SLBT_DRIVER_MODULE)
		    ? "" : dctx->cctx->settings.dsoprefix,
		dctx->cctx->libname,

		/* current, age, revision */
		fvernum ? dctx->cctx->verinfo.major : current,
		fvernum ? dctx->cctx->verinfo.minor : age,
		fvernum ? dctx->cctx->verinfo.major : revision,

		/* installed, shouldnotlink */
		"no","no",

		/* dlopen, dlpreopen */
		"","",

		/* libdir */
		dctx->cctx->rpath ? dctx->cctx->rpath : "");

	return (ret <= 0) || fclose(fout)
		? SLBT_SYSTEM_ERROR(dctx)
		: 0;
}

int  slbt_create_library_wrapper(
	const struct slbt_driver_ctx *	dctx,
	struct slbt_exec_ctx *		ectx,
	char *				arname,
	char *				soname,
	char *				soxyz,
	char *				solnk)
{
	if (dctx->cctx->drvflags & SLBT_DRIVER_LEGABITS)
		return slbt_create_compatible_library_wrapper(
			dctx,ectx,arname,soxyz,soname,solnk);
	else
		return slbt_create_default_library_wrapper(
			dctx,ectx,arname,soxyz,soname,solnk);
}
