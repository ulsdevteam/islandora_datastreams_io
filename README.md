Islandora Datastreams Input/Output & Object Relationships
=============
This module is mostlly just a web-ui wrapper for the islandora_datastream_crud module, but also offers a method for adding or removing particular relationships for a set of islandora objects.

Since this module shells out to the drush commands from islandora_datastream_crud be sure to have the latest version of that module installed.

### Installing
There are no options to configure for this module.  Once this module is installed, users can be configured to have permission to Import / Export datastreams using the "Use the datastreams import and export tool" `ISLANDORA_DATASTREAMS_IO` permission.  The export form is located /islandora/datastreams_io/export and the import form is located /islandora/datastreams_io/import.

### Using from other modules
The PID values can be passed from other modules by calling the islandora_datastreams_io_pids_to_export_form function that is provided in the main module code (see **Fetch Methods** below for the possible fetch constants).  To use this from anywhere, simply add the two lines:
```php
  module_load_include('module', 'islandora_datastreams_io');
  // Calling this will populate the PIDS for and redirect to the export form.
  islandora_datastreams_io_pids_to_export_form($pids, ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS);
```
This function will redirect to the export form and pre-load based on the `$pids` and the `$pids_fetch_method` values.
If calling the form this way and the pids_fetch_method is set to `ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS` "List of PIDS", the PIDS field will be made read-only.

## Exporting Datastreams (Output)
Select the specific datastream and define a list of objects (see **Fetch Methods** below), and download a zip file that contains files corresponding to the objects' datastream that was selected.

*If the files are intended to be Imported back into the system, **DO NOT CHANGE the filenames**.*

### Datastream selection
The select box is populated with the names of all datastreams that are in use for the current installation.  The value in parenthesis beside the datastream identifier (DSID) is the number of objects that have that datastream.

### Fetch methods
The following constands are defined:
`ISLANDORA_DATASTREAMS_IO_FETCH_SOLR` - Will return objects that match based on the Solr query.  The value of the Solr query here should simply be the query value (eg: `mods_genre_s\:photograph`)  and not include any filter parameters, any special Solr functions.
`ISLANDORA_DATASTREAMS_IO_FETCH_LISTPIDS` - This will return objects that match the list of PID values.  Simply enter a PID value on each line.
`ISLANDORA_DATASTREAMS_IO_FETCH_COLLECTION` - This will return all objects that are members of a given collection.
`ISLANDORA_DATASTREAMS_IO_FETCH_MODEL` - This will return all objects that are of a given model.

### Adaptive options

This module can extend the options based on what is installed.  In other words, if 
islandora_solr is installed, provide the option to import / export using a Solr query.

## Importing Datastreams (Input)
Given the ZIP file from the **Exporting Datastreams** step, the files can be manipulated by a third-party program and then zipped back up to import back into the system.  Simply navigate to the import page at and upload the zip file.  The form will inspect the ZIP file to determine the objects and specific datastream identifier (DSID) and prompt the user whether or not to import the files that are in the ZIP file.  

## Adding / Removing Relationships
This feature allows a specific relationship to either be added or removed from a set of objects.  In order to use this, several parameters need to be provided.  These values should be familiar to developers because they correspond to two thirds of the triples that set the relationship to the object.  The other value stores the namespace related to the relationship ontology.  After this process runs, the status of the relationship updates are displayed on the screen.  The user should test that their relationships exist as they intended.

For example, the "isMemberOfSite" relationship is related to the namespace value of "http://digital.library.pitt.edu/ontology/relations#".

The relationship is skipped when it exists already in cases where it is being added, or if it does not exist in cases where it is being removed.

**IMPORTANT:** Do not change the filenames because the system needs the filename in order to know which object and datastream to replace.

## Author / License

Written by Brian Gillingham for the [University of Pittsburgh](http://www.pitt.edu).  Copyright (c) University of Pittsburgh.

Released under a license of GPL v2 or later.
