.. YipLoader documentation master file, created by
   sphinx-quickstart on Sun Jun  7 04:21:43 2026.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

=======================
YipLoader Documentation
=======================

This is the mod developer documentation for YipLoader, a generic mod loader for
Android games that provides a minimal but functional API.

.. note:: YipLoader was split from KnShim's loading functionality and is
   currently undergoing changes to become a generic, useful mod loader. While it
   should work, it is missing certian functionality like dependency validation.

.. warning:: Overuse of YipLoader may result in death and destruction.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

Getting Started
===============

.. note:: The developer exprience is currently quite rough and will improve in
   the future.

As far as prerequsites go, please install any version of the Android NDK, and
optionally add the NDK to your path.

To automatically generate the basic structure of a mod project, open a terminal
in an empty directory where you wish to store your project, then use the
YipLoader SDK to generate a project:

   python <path to SDK directory> create

You will be asked for some basic information about your mod, then your mod files
will be generated.

Data Types
==========

.. c:struct:: YipBuffer

   Provides a structured way of storing and passing byte buffers to and from functions.

   .. c:member:: size_t size
   
   Size of the data, in bytes.
   
   .. c:member:: uint8_t *data
   
   The data as a byte pointer

   .. note:: If :c:member:`data` is ``NULL``, then :c:member:`size` must be zero. 

.. c:struct:: YipModInfo

   Structure which describes metadata about a mod. This is mainly used in ``mod_info.c`` to describe information about your mod, for example::
   
      YipModInfo mod_info = {
         .name = "Sprite Sheet Extender",
         .author = "KD",
         .description = "Extends the spritesheet from 64 to 256 tiles",
         .game = "smashhit",
         .version = 1,
         .assumes = &yiploader_version,
         .conflicts = NULL,
      };
   
   When using a mod structure as a predicate (e.g. used to compare against for the purposes of dependency and conflict checking) it can be a partial mod info structure: that is, not all fields need to be specified and can be NULL, in which case they are treated as a wildcard.
   
   The assumes and conflicts fields are linked lists of partial mod structures describing mods that the current mod depends upon or conflicts with the presence of, respectively. Note that the current version of YipLoader does not try to load mods in the correct order or check for conflicts, so these currently only exist for the sake of forwards compatibility. 
   
   .. c:member:: const char *name
   
      Name of your mod as any string. Make sure it stays consistent across versions.
   
   .. c:member:: const char *author
   
      Informal author name
   
   .. c:member:: const char *description
   
      Informal description of what your mod does
   
   .. c:member:: const char *game
   
      The game your mod is for, or ``NULL`` if it is for all games
   
   .. c:member:: uint32_t version
      
      The version of your mod, represented as a 32-bit integer.
   
   .. c:member:: YipModInfo *assumes
      
      List of partial mod infos that this mod depends on
   
   .. c:member:: YipModInfo *conflicts
   
      List of partial mod infos that this mod conflicts with
   
   .. c:member:: YipModInfo *next
      
      .. warning:: Do not modify this yourself.
      
      Next mod in the chain

   .. note:: This structure may contain more fields; however, they are internal to YipLoader and should generally not be accessed by mods.

.. c:type:: void (*YipModConstructor)(void)
   
   .. note:: You will probably never need to reference this type directly.
   
   This is the interface of the ``mod_init()`` function.


Functions
=========

function info

.. c:function:: void *YipLookupSymbol(const char *symbol)
   
   Finds the loaded address of the named symbol for the main game binary and returns it. If it isn't found, this returns ``NULL``.
   
   :param symbol: The symbol name to lookup
   :returns: Pointer to the object named by the symbol
   :retval NULL: When the symbol is not found

.. c:function:: void *YipHookFunction(const char *symbol, void *hook, bool replace)

   This creates a hook or replacement for the function with the given symbol name. The hook argument should be a pointer to a function that implements the same interface (i.e. arguments and return values) as the original function and wraps or replaces it.

   When replace is false, a special function pointer is returned which can be called to invoke the original function. When it is true, however, the original function is assumed to be totally replaced by the hook and is no longer accessable; in that case, hook itself is returned.

   When hooking fails, this function returns NULL.
   
   :param symbol: The symbol name associated with the function to hook or replace
   :param hook: The function which hooks or replaces the original function
   :param replace: :c:expr:`true` when the original function does not need to be called, or :c:expr:`false` when the original function needs to be called and a pointer to call it returned on success
   :returns: A pointer by which the original function may be called, if it has not be replaced. Otherwise, this is the result of the symbol lookup.
   :retval NULL: On any failure
   :retval non-NULL: On success
   
.. c:function:: void *YipHookFunctionAt(size_t vaddr, void *hook, bool replace)
   
   This is the same as :c:func:`YipHookFunction`, except it takes the virtual address instead of a symbol name.
   
   .. note:: It's not recommended to use this unless you are hooking a static function.
   
.. c:function:: bool YipPatch(size_t vaddr, YipBuffer buffer)

   Patch a specific chunk of memory, given the buffer to replace it with and the virtual address of that chunk. The chunk of memory must be within the game binary and is given relative to the ELF virtual base address.
   
   .. tip:: If what you are doing is possible using hooks, please use them instead as they tend to be more version-agnostic and don't break as much between upgrades. 
   
.. c:function:: const char *YipGetGameName(void)

   Get the name of the currently loaded app. This is the same as the name of the game's main library, e.g. libsmashhit.so results in a game name of smashhit.

.. c:function:: struct android_app *YipGetAndroidAppStruct(void)

   Get the struct android_app instance passed to android_main() at app startup.

.. c:function:: Leaf *YipGetLeafInstance(void)

   Get the instance of Leaf used to load the game's main binary.

.. c:function:: const YipModInfoStatic *YipGetModList(void)

   Get the mod at the head of the internal loaded mod list. 
   
Add your content using ``reStructuredText`` syntax. See the
`reStructuredText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_
documentation for details.




