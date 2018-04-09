Islandora Datastreams Input/Output & Object Relationships
=============
This module started out as a web-ui wrapper for the [Islandora Datastream CRUD](https://github.com/mjordan/islandora_datastream_crud) module, but also offers a method for adding or removing particular relationships for a set of islandora objects;  be sure to have the latest version of that module installed.  Additional functionality has been added to do more than simply import / export datastreams.  The module now supports adding / removing relationships as well as xsl transforms.  

This module also makes use of the [Islandora Solr Search](https://github.com/islandora/islandora_solr_search) module if it is installed to provide object selection by Solr query, by Collection, and by Model -- it is recommended that Islandora Solr Search be installed.

**NOTE:** The module processes these updates in a loop that is determined by the list of PID values.  If there are more than a hundred objects to process, expect that it will take at least a minute to process the.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Datastream CRUD](https://github.com/mjordan/islandora_datastream_crud)
* [Islandora Solr Search](https://github.com/islandora/islandora_solr_search) (optionally)

## Installing
There are no options to configure for this module.  Once this module is installed, users can be configured to have permission to Import / Export datastreams using the "Use the datastreams import and export tool" `ISLANDORA_DATASTREAMS_IO` permission.  

# Configuration
Configuration for this module /admin/islandora/datastreams_io also serves as the launching point for the various operating modes.

The latest version will support schema tests for imported MODS datastreams as well as provides a hook that can be used in coding a schema check within any user-developed module (see "Writing a custom hook_mods_schema_check" below).

The **Solr query limit** allows control over the upper limit of objects that can be returned by any Solr query (whether "Collection" or "Model" is selected).  The default value should be fine for all cases, but there may be a case where a larger number is need. 

**MODS to DC XSL Transform** is pending development.  When this is done, it will make a DC derivative based on the selected transform ANY time the resultant datastream is a MODS (could be Import or XSLT Transform mode).
**Update Object label from MODS titleInfo/title** is also pending development.

# Operating modes
1. Export Datastreams as a ZIP file `/islandora/datastreams_io/export`
2. Import ZIP file containing Datastreams `/islandora/datastreams_io/import`
3. Add/Remove relationships for objects `/islandora/datastreams_io/relationships`
4. Modify or Copy datastream optionally using an xslt transformation `/islandora/datastreams_io/transform`

## Export Datastreams as a ZIP file (Output)
Select the specific datastream and define a list of objects (see **Fetch Methods** below), and download a zip file that contains files corresponding to the objects' datastream that was selected.

*If the files are intended to be Imported back into the system, **DO NOT CHANGE the filenames**.*

## Import ZIP file containing Datastreams (Input)
Given the ZIP file from the **Exporting Datastreams** step, the files can be manipulated by a third-party program and then zipped back up to import back into the system.  Simply navigate to the import page at and upload the zip file.  The form will inspect the ZIP file to determine the objects and specific datastream identifier (DSID) and prompt the user whether or not to import the files that are in the ZIP file.

When any MODS datastreams are imported, it may be checked against one of the MODS version 3.x schemas (see section above on configuration).

## Add/Remove relationships for objects
This feature allows a specific relationship to either be added or removed from a set of objects.  In order to use this, several parameters need to be provided.  These values should be familiar to developers because they correspond to two thirds of the triples that set the relationship to the object.  The other value stores the namespace related to the relationship ontology.  After this process runs, the status of the relationship updates are displayed on the screen.  The user should test that their relationships exist as they intended.

In order to make the relationship, the **predicate**, **namespace**, and **value** must be provided to make the triples relationship for the object (for each PID value provided to the set).

For example, the "isMemberOfSite" relationship ontology is related to the namespace of "http://digital.library.pitt.edu/ontology/relations#".

The relationship is skipped when it exists already in cases where it is being added, or if it does not exist in cases where it is being removed.

## Modify or Copy datastream optionally using an xslt transformation
This method will allow a datastream to be transformed using an XSLT transform file.  This will only work on datastreams that are text/xml.  Also, the XSLT transform must be valid.

Additionally, a datastream could be copied to a new  datastream identifier without any transform.  We needed to copy a large number of OBJ datastreams to be PDF datastreams.  In order to do this, we simply selected the PID values (using "List of PID values" for the Select Objects mode), selected the OBJ datastream as the source to transform, skipped the transform option, and set the destination datastream to PDF. 

### Using from other modules
The PID values can be passed from other modules by calling the islandora_datastreams_io_pids_to_export_form function that is provided in the main module code (see **Fetch Methods** below for the possible fetch constants).  To use this from anywhere, simply add the two lines:
```php
  module_load_include('module', 'islandora_datastreams_io');
  // Calling this will populate the PIDS for and redirect to the export form.
  islandora_datastreams_io_pids_to_export_form($pids, ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS);
```
This function will redirect to the export form and pre-load based on the `$pids` and the `$pids_fetch_method` values.
If calling the form this way and the pids_fetch_method is set to `ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS` "List of PIDS", the PIDS field will be made read-only.

### Datastream selection
The select box is populated with the names of all datastreams that are in use for the current installation.  The value in parenthesis beside the datastream identifier (DSID) is the number of objects that have that datastream.

### Fetch methods
The following constands are defined:
`ISLANDORA_DATASTREAMS_IO_FETCH_SOLR` - Will return objects that match based on the Solr query.  The value of the Solr query here should simply be the query value (eg: `mods_genre_s\:photograph`)  and not include any filter parameters, any special Solr functions.
`ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS` - This will return objects that match the list of PID values.  Simply enter a PID value on each line.
`ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION` - This will return all objects that are members of a given collection.
`ISLANDORA_DATASTREAMS_IO_FETCH_MODEL` - This will return all objects that are of a given model.


### Writing a custom hook_mods_schema_check
It is possible to have multiple modules handle this hook.  Each module that does have code for this hook_mods_schema_check will return a boolean result -- which is combined with the results of any other modules that code for it -- as well as the selected MODS Schema would also run and potentially make any MODS pass/fail.  If you ONLY want to perform the schema validation with a custom module, select "hook_mods_schema_check ONLY" for the **MODS Schema** value in the configuration.

### Adaptive options

This module can extend the options based on what is installed.  In other words, if islandora_solr is installed, provide the option to import / export using a Solr query.

**IMPORTANT:** Do not change the filenames because the system needs the filename in order to know which object and datastream to replace.


## Author / License

Written by Brian Gillingham for the [University of Pittsburgh](http://www.pitt.edu).  Copyright (c) University of Pittsburgh.

Released under a license of GPL v2 or later.
