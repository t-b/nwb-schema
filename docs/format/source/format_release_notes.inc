Release Notes: NWB Format
=========================

Proposed Future Changes
-----------------------
Issue for proposed enhancements for NWB 2.x are managed in the GitHub project https://github.com/orgs/NeurodataWithoutBorders/projects/2 .

Proposals for future enhancements are currently managed via issues on GitHub https://github.com/NeurodataWithoutBorders/nwb-schema/issues
and should be assigned to the project ``NWB 2.x Proposed Enhancements``. Here a few format-specific enhancements that have been proposed:

    - Create new module to store frequency decompositions: see https://github.com/NeurodataWithoutBorders/nwb-schema/issues/46
    - Create new module to store matrix factorizations: see https://github.com/NeurodataWithoutBorders/nwb-schema/issues/47
    - Change small metadata datasets to attributes where approbritate: see https://github.com/NeurodataWithoutBorders/nwb-schema/issues/48
    - Addition of Interval series to SpikeWaveform and ElectrodeGroup objects: https://github.com/NeurodataWithoutBorders/nwb-schema/issues/2


2.0.0 Alpha (August 2017)
-------------------------

Improved organization of electrode metadata in ``/general/extracellular_ephys``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change:** Consolidate metadata from related electrodes (e.g., from a single device) in a single location.

**Example:** Previous versions of the format specified in ``/general/extracellular_ephys`` for each electrode a
group ``<electrode_group_X>`` that stores 3 text datastes with a description, device name, and location, respectively.
The main ``/general/extracellular_ephys group`` then contained in addition the following datasets:

    - ``electrode_group`` text array describing for each electrode_group (implicitly referenced by index)
      which device (shank, probe, tetrode, etc.) was used,
    - ``electrode_map`` array with the x,y,z locations of each electrode
    - ``filtering``, i.e., a single string describing the filtering for all electrodes (even though each
      electrode might be from different devices), and iv),
    - ``impedance``, i.e, a single text array for impedance (i.e., the user has to know which format the
      string has, e.g, a float or a tuple of floats for impedance ranges).


**Reason:**

    - This change  allows us to replace arrays with implicit links (define via integer indicies) in, e.g.,
      ``<ElectricalSeries>`` and ``<FeatureExtraction>`` with a single explicit link to an ``<ElectrodeGroup>``
    - Avoid explosion of the number of groups and datasets. For example, in the case of an ECoG grid with 128 channels
      one had to create 128 groups and corresponding datasets to store the required metadata about the electrodes
      using the original layout.
    - Avoid mixing of metadata from multiple devices (sessions etc.), e.g., the ``electrode_map`` would store
      the physical location for all electrodes used in the file (indpendent of which device, subject, etc. the
      electrodes are associated with).
    - Simplify access to related metadata. E.g., access to metadata from all electrodes of a single device requires
      resolution of a potentially large number of implicit links and access to a large number of groups (one per electrode)
      and datasets.
    - Improve performance of metadata access operations. E.g., to access the ``location`` of all electrodes corresponding to a
      single recording in an ``<ElectricalSeries>`` in the original layout required iterating over a potentially large number of
      groups and datasets (one per electrode), hence, leading to a large number of small, independent read/write/seek operations,
      causing slow performance on common data accesses. Using the new layout, these kind of common data accesses can often be
      resolved via a single read/write
    - Ease maintance, use, and development through consolidation of related metadata

**Format Changes**

    - Added specification of a new neurodata type ``<ElectrodeGroup>`` group.
      Each ``<ElectrodeGroup>`` contains the following datasets to describe the metadata of a set of related
      electrodes (e.g,. all electrodes from a single device):

        - ``description`` : text dataset (for the group)
        - ``device``: Soft link to the device in ``/general/devices/``
        - ``location``: Text description of the location of the device
        - ``channel_description``: Array with text dataset description of each electrode
        - ``channel_location``: Array with text description of the location of each channel
        - ``channel_coordinates`` : Physical location of the electrodes [num_electrodes, x, y, z]
        - ``channel_filtering`` : Dataset describing the filtering applied
        - ``channel_impedance`` : Float array with the imedance values for the electrodes. This may be a 2D array
          of ``[num_electrodes,2]`` in case impedance values are stored as ranges.

    - Updated ``/general/extracellular_ephys`` as follows:

        - Replaced ``/general/extracellular_ephys/<electrode_group_X>`` group (and all its contents) with the new ``<ElectrodeGroup>``
        - Removed ``/general/extracellular_ephys/electrode_map`` dataset. This information is now stored in ``<ElectrodeGroup>/channel_coordinates``
        - Removed ``/general/extracellular_ephys/electrode_group`` dataset. This information is now stored in ``<ElectrodeGroup>/device``.
        - Removed ``/general/extracellular_ephys/impedance`` dataset. This information is now stored in ``<ElectrodeGroup>/impedance``.
        - Removed ``/general/extracellular_ephys/filtering`` dataset. This information is now stored in ``<ElectrodeGroup>/filtering``.

Replaced Implicit Links/Data-Structures with Explicit Links
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change** Replace implicit links with explicit soft-links to the corresponding HDF5 objects where possible, i.e.,
use explicit HDF5 mechanisms for expressing basic links between data rather than implicit ones that require
users/developers to know how to use the specific data.

**Reason:** In several places datasets containing arrays of either i) strings with object names, ii) strings with paths,
or iii) integer indexes are used that implicitly point to other locations in the file. These forms of implicit
links are not self-describing (e.g., the kind of linking, target location, implicit size and numbering assumptions
are not easily identified). This hinders human interpretation of the data as well as programmatic resolution of these
kind of links.

**Format Changes:**

    - Text dataset ``image_plane`` of ``<TwoPhotonSeries>`` is now a link to the corresponding ``<ImagingPlane>``
      (which is stored in ``/general/optophysiology``)
    - Text dataset ``image_plane_name`` of ``<ImageSegmentation>`` is now a link to the corresponding ``<ImagingPlane>``
      (which is stored in ``/general/optophysiology``). The dataset is also renamed to ``image_plane`` for consistency with ``<TwoPhotonSeries>``
    - Text dataset ``electrode_name`` of ``<PatchClampSeries>`` is now a link to the corresponding ``<IntracellularElectrode>``
      (which is stored in ``/general/intracellular_ephys``). The dataset is also renamed to ``electrode`` for consistency.
    - Text dataset ``site`` in ``<OptogeneticSeries>`` is now a link to the corresponding ``<StimulusSite>``
      (which is stored in ``/general/optogenetics``).
    - Integer dataset ``electrode_idx`` of ``FeatureExtraction`` is now a link to the corresponding ``<ElectrodeGroup>``
      (which is stored in ``/general/extracellular_ephys``). Also, renamed the dataset to ``electrode_group`` for consistency.
    - Integer array dataset ``electrode_idx`` of ``<ElectricalSeries>`` is now a link to the corresponding ``<ElectrodeGroup>``
      (which is stored in ``/general/extracellular_ephys``). Also, renamed the dataset to ``electrode_group`` for consistency.
    - Text dataset ``/general/extracellular_ephys/<electrode_group_X>/device`` is now a link ``<ElectrodeGroup>/device``

Added missing metadata
^^^^^^^^^^^^^^^^^^^^^^

**Change:** Add a few missing metadata attributes/datasets.

**Reason:** Ease data interpretation, improve format consistency, and enable storage of additional metadata

**Format Changes:**

    - ``/general/devices`` text dataset becomes group with neurodata type ``Device`` to enable storage of more complex
      and structured metadata about devices (rather than just a single string)
    - Added attribute ``unit=Seconds`` to ``<EventDetection>/times`` dataset to explicitly describe time units
      and improve human and programmatic data interpretation
    - Added ``filtering`` dataset to type ``<IntracellularElectrode>`` (i.e., ``/general/intracellular_ephys/<electrode_X>``)
      to enable specification of per-electrode filtering data


Improved identifiably of objects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change:** All groups and datasets are now required to either have a unique ``name`` or a unique ``neurodata_type`` defined.

**Reason:**  This greatly simplifies the unique identification of objects with variable names.

**Format Changes:** Defined missing neurodata_types for a number of objects, e.g.,:

    - Group ``/general/optophysiology/<imaging_plane_X>`` now has the neurodata type ``ImagingPlane``
    - Group ``/general/intracellular_ephys/<electrode_X>`` now has the neurodata type ``IntracellularElectrode``
    - Group ``/general/optogenetics/<site_X>`` now has the neurodata type ``StimulusSite``
    - ...

Improved Consistency
^^^^^^^^^^^^^^^^^^^^

**Change:** Rename objects (and add missing objects)

**Reason:** Improve consistency in the naming of data objects that store similar types of information in different
places and ensure that the same kind of information is available.

**Format Changes:**

    - Added missing ``help`` attribute for ``<BehavioralTimeSeries>`` to improve consistency with other types
      as well as human data interpretation
    - Renamed dataset ``image_plan_name`` in ``<ImageSegmentation>`` to ``image_plane``to ensure consistency
      in naming with ``<TwoPhotonSeries>``
    - Renamed dataset ``electrode_name`` in ``<PatchClampSeries>`` to ``electrode`` for consistency (and
      since the dataset is now a link, rather than a text name).
    - Renamed dataset ``electrode_idx`` in ``<FeatureExtraction>`` to ``electrode_group`` for consistency
      (and since the dataset is now a link to the ``<ElectrodeGroup>``)
    - Renamed dataset ``electrode_idx`` in ``<ElectricalSeries>`` to ``electrode_group`` for consistency
      (and since the dataset is now a link to the ``<ElectrodeGroup>``)

Improved governance and accessibility
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change:** Updated release and documentation mechanisms for the NWB format specification

**Reason:** Improve governance, ease-of-use, extensibility, and accessibility of the NWB format specification

**Specific Changes**

    - The NWB format specification is now released in seperate Git repository
    - Format specifications are released as YAML files (rather than via Python .py file included in the API)
    - Organized core types into a set of smaller YAML files to ease overview and maintenance
    - Converted all documentation documents to Sphinx reStructuredText to improve portability, maintainability,
      deployment, and public access
    - Sphinx documentation for the format are auto-generated from the YAML sources to ensure consistency between
      the specification and documentation
    - The pyNWB API now provides dedicated data structured to interact with NWB specifications, enabling users
      programmatically access and generate format specifications

Improved organization of processed data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change:** Relaxed requirements and renamed core types used for storage of processed data.

**Reason:** Ease user intuition and provide greater flexibility for users.

**Specific Changes:** The following changes have been made to the organization of processed data:

    * *Module* has been renamed to *ProcessingModule* to avoid possible confusion and to clarify its purpose
    * *Interface* has been renamed to *NWBContainer* to avoid confusion and ease intuition
    * *NWBContainer* is now the base type of all main data storage types in the NWB format, including *TimeSeries.
      This also means that all data storage types now inherit the required *help* and *source* attribute from
      *NWBContainer*. Another implication of this change is that it is now possible to define data processing
      types that directly inherit from *TimeSeries*.
    * All *Interface* types in the orignal format had fixed names. The fixed names have been replaced by
      specification of corresponding default names in the current format. This changes enables storage of
      multiple instances of the same analysis type in the same *ProcessingModule* by allowing users to
      customize the name of the data processing types, whereas in version 1.0.x only a single instance of
      each analysis could be stored in a *ProcessingModule* due to the requirement for fixed names.

Ancestry
^^^^^^^^

**Change:** Removed the explicit specification of ancestry as an attribute as part of the format specification

**Reason:** 1) avoid redundant information as the ancestry is encoded in the inheritance of types, 2) ease maintenance,
and 3) avoid possible inconsistencies between the ancestry attribute and the true ancestry (i.e., inheritance hierarchy)
as defined by the spec.

**Note** The new specification API as part of PyNWB/FORM makes the ancestry still easily accessible to users. As
the ancestry can be easily extracted from the spec, we currently do not write a separate ancestry attribute
but this could be easily added if needed.


Specification language changes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Change:** Numerous changes have been made to the specification language itself in NWB 2.0. Most changes to
the specification language effect mainly how the format is specified, rather than the actual structure of the format.
The changes that have implications on the format itself are described next. For an overview and discussion of the
changes to the specification language see `specification language release notes <http://schema-language.readthedocs.io/en/latest/specification_language_release_notes.html#release-notes>`_.

Specification of dataset dimensions
"""""""""""""""""""""""""""""""""""

**Change:** Updated the specification of the dimensions of dataset

**Reason:** To simplify the specification of dimension of datasets and attribute

**Format Changes:**

    * The shape of various dataset is now specified explicitly for several datasets via the new ``shape`` key
    * The ``unit`` for values in a dataset are specified via an attribute on the dataset itself rather than via
      ``unit`` definitions in structs that are available only in the specification itself but not the format.
    * In some cases the length of a dimension was implicitly described by the length of structs describing the
      components of a dimension. This information is now explitily described in the ``shape`` of a dataset.

Added ``Link`` type
"""""""""""""""""""

**Change** Added new type for links

**Reason:**

    - Links are usually a different type than datasets on the storage backend (e.g., HDF5)
    - Make links more readily identifiable
    - Avoid special type specification in datasets

**Format Changes:** The format itself is not affected by this change aside from the fact that
datasets that were links are now explicitly declared as links.


Removed datasets defined via autogen
""""""""""""""""""""""""""""""""""""

**Change** Support for ``autogen`` has been removed from the specification language. After review
of all datasets that were produced via autogen it was decided that all autogen datasets should be
removed from the format.

**Reason** The main reasons for removal of autogen dataset is to ease use and maintance of NWB files by
i) avoiding redundant storage of information (i.e., improve normalization of data) and ii) avoiding
dependencies between data (i.e., datasets havging to be updated due to changes in other locations in a file).

**Format Changes**

* Datasets/Attributes that have been removed due to redundant storage of the path of links stored in the same group:

    * IndexSeries/indexed_timeseries_path
    * RoiResponseSeries/segmentation_interface_path
    * ImageMaskSeries/masked_imageseries_path
    * ClusterWaveforms/clustering_interface_path
    * EventDetection/source_electricalseries_path
    * MotionCorrection/image_stack_name/original_path
    * NWBFile/epochs/epoch_X.links

* Datasets//Attributes that have been removed because they stored only a list of groups/datasets (of a given type or property)
  in the current group.

    * Module.interfaces  (now ProcessingModule)
    * ImageSegmentation/image_plane/roi_list
    * UnitTimes/unit_list
    * TimeSeries.extern_fields
    * TimeSeries.data_link
    * TimeSeries.timestamp_link
    * TimeSeries.missing_fields


* Other datasets/attributes that have been removed to ease use and maintance because the data stored is redundant and can be
  easily extracted from the file:

    * NWBFile/epochs.tags
    * TimeSeries/num_samples
    * Clustering/cluster_nums


Removed ``'neurodata\_type=Custom'``
""""""""""""""""""""""""""""""""""""

**Change** The ``'neurodata\_type=Custom'`` has been removed.

**Reason** All additions of data should be governed by extensions. Custom datasets can be identified based on
the specification, i.e., any objects that are not part of the specification are custom.



1.0.6, April 8, 2017
--------------------
Minor fixes:

    * Modify <IntervalSeries>/ documentation to use html entities for < and >.
    * Fix indentation of unit attribute data_type, and conversion attribute description in
      ``/general/optophysiology/<imaging_plane_X>/manifold``.
    * Fix typos in ``<AnnotationSeries>/`` conversion, resolution and unit attributes.
    * Update documentation for ``IndexSeries`` to reflect more general usage.
    * Change to all numerical version number to remove warning message when installing using setuptools.

1.0.5i_beta, Dec 6, 2016
------------------------
Removed some comments. Modify author string in info section.

1.0.5h_beta, Nov 30, 2016
-------------------------
Add dimensions to ``/acquisition/images/<image_X>``


1.0.5g\_beta, Oct 7, 2016
-------------------------

-  Replace group options: ``autogen: {"type": "create"}`` and ``"closed": True``
   with ``"\_properties": {"create": True}`` and ``"\_properties": {"closed": True}``.
   This was done to make the specification language more consistent by
   having these group properties specified in one place (``"\_properties"``
   dictionary).


1.0.5f\_beta, Oct 3, 2016
-------------------------

-  Minor fixes to allow validation of schema using json-schema specification
   in file ``meta-schema.py`` using utility ``check\_schema.py``.


1.0.5e\_beta, Sept 22, 2016
---------------------------

-  Moved definition of ``<Module>/`` out of ``/processing`` group to allow creating subclasses of Module.
   This is useful for making custom Module types that specified required interfaces. Example of this
   is in ``python-api/examples/create\_scripts/module-e.py`` and the extension it uses (``extensions/e-module.py``).
-  Fixed malformed html in ``nwb\_core.py`` documentation.
-  Changed html generated by ``doc\_tools.py`` to html5 and fixed so passes validation at https://validator.w3.org.

1.0.5d\_beta, Sept 6, 2016
--------------------------

- Changed ImageSeries img\_mask dimensions to: ``"dimensions": ["num\_y","num\_x"]`` to match description.

1.0.5c\_beta, Aug 17, 2016
--------------------------

- Change IndexSeries to allow linking to any form of TimeSeries, not just an ``ImageSeries``


1.0.5b\_beta, Aug 16, 2016
--------------------------

-  Make ``'manifold'`` and ``'reference\_frame'`` (under
   ``/general/optophysiology``) recommended rather than required.
-  In all cases, allow subclasses of a TimeSeries to fulfill validation
   requirements when an instance of TimeSeries is required.
-  Change unit attributes in ``VoltageClampSeries`` series datasets from
   required to recommended.
-  Remove ``'const'=True`` from ``TimeSeries`` attributes in ``AnnotationSeries``
   and ``IntervalSeries``.
-  Allow the base ``TimeSeries`` class to store multi-dimensional arrays in
   ``'data'``. A user is expected to describe the contents of 'data' in the
   comments and/or description fields.


1.0.5a\_beta, Aug 10, 2016
--------------------------

Expand class of Ids allowed in ``TimeSeries`` ``missing\_fields`` attribute to
allow custom uses.


1.0.5\_beta Aug 2016
--------------------

-  Allow subclasses to be used for merges instead of base class
   (specified by ``'merge+'`` in format specification file).
-  Use ``'neurodata\_type=Custom'`` to flag additions that are not describe
   by a schema.
-  Exclude TimeSeries timestamps and starting time from under
   ``/stimulus/templates``


1.0.4\_beta June 2016
---------------------

- Generate documentation directly from format specification file."
- Change ImageSeries ``external\_file`` to an array.
- Made TimeSeries description and comments recommended.

1.0.3 April, 2016
-----------------

- Renamed ``"ISI\_Retinotopy"`` to ``"ISIRetinotopy"``
- Change ``ImageSeries`` ``external\_file`` to an array. Added attribute ``starting\_frame``.
- Added ``IZeroClampSeries``.


1.0.2 February, 2016
--------------------

-  Fixed documentation error, updating ``'neurodata\_version'`` to ``'nwb\_version'``
-  Created ``ISI\_Retinotopy`` interface
-  In ``ImageSegmentation`` module, moved ``pix\_mask::weight`` attribute to be its
   own dataset, named ``pix\_mask\_weight``. Attribute proved inadequate for
   storing sufficiently large array data for some segments
-  Moved ``'gain'`` field from ``Current/VoltageClampSeries`` to parent
   ``PatchClampSeries``, due need of stimuli to sometimes store gain
-  Added Ken Harris to the Acknowledgements section


1.0.1 October 7th, 2015
-----------------------

-  Added ``'required'`` field to tables in the documentation, to indicate if
   ``group/dataset/attribute`` is required, standard or optional
-  Obsoleted ``'file\_create\_date'`` attribute ``'modification\_time'`` and made ``file\_create\_date`` a text array
-  Removed ``'resistance\_compensation'`` from ``CurrentClampSeries`` due being duplicate of another field
-  Upgraded ``TwoPhotonSeries::imaging\_plane`` to be a required value
-  Removed ``'tags'`` attribute to group 'epochs' as it was fully redundant with the ``'epoch/tags'`` dataset
-  Added text to the documentation stating that specified sizes for integer
   values are recommended sizes, while sizes for floats are minimum sizes
-  Added text to the documentation stating that, if the
   ``TimeSeries::data::resolution`` attribute value is unknown then store a ``NaN``
-  Declaring the following groups as required (this was implicit before)

.. code-block:: python

    acquisition/

    \_ images/

    \_ timeseries/

    analysis/

    epochs/

    general/

    processing/

    stimulus/

    \_ presentation/

    \_ templates/


This is to ensure consistency between ``.nwb`` files, to provide a minimum
expected structure, and to avoid confusion by having someone expect time
series to be in places they're not. I.e., if ``'acquisition/timeseries'`` is
not present, someone might reasonably expect that acquisition time
series might reside in ``'acquisition/'``. It is also a subtle reminder
about what the file is designed to store, a sort of built-in
documentation. Subfolders in ``'general/'`` are only to be included as
needed. Scanning ``'general/'`` should provide the user a quick idea what
the experiment is about, so only domain-relevant subfolders should be
present (e.g., ``'optogenetics'`` and ``'optophysiology'``). There should always
be a ``'general/devices'``, but it doesn't seem worth making it mandatory
without making all subfolders mandatory here.


1.0.0 September 28\ :sup:`th`, 2015
-----------------------------------

- Convert document to .html
- ``TwoPhotonSeries::imaging\_plane`` was upgraded to mandatory to help
  enforce inclusion of important metadata in the file.

