The GitQlient 1.0.0 release contains a deep refactor and new features.

A more detailed list of changes follows below.

Big features:
- **Multi-repository support**: Added support to have multiple repositories opened at the same time.
- **Submodules**: Added support for opening, adding and updating the submodules in a repository. The submodules can be opened as a normal repository to work with them.
- **Blame & History**: There is a whole new screen where one can review the history of a file and blame it.

Minor features:
- **Branches**: The branches are shown in a tree structure, more easy to follow.
- **Diffs**: Between files, between random commits, etc.
- **Patches**: I took back the patches but with some improvements: Multi-selection of commits to export them. Possibility of applying patches or import them.
- **Cloning/Init repositories**: Progress dialogs when cloning or opening really big repos.
- **Refactor**: The old backend has been reworked in a 97%. There are some pending changes that will be done on the v2.0.0.

Other technical improvements from the version available in GitQlientPlugin v1.0.0:
- Re-organization of the project structure for a more intuitive architecture
- Controls:
   - Main view buttons display the current view.
   - Added Diff as option
- Repository view re-structure (aka HistoryWidget):
   - "Go to SHA" featured integrated in the repository view
- Blame improvements:
   - Adding tabs to have several blames and improving navigation between the commit history and the blames
- New Diff view:
   - Adding options to switch between different diffs (buttons list)
   - Added information about the commits that are compared and the files that are modified
- Added option to reset staged files
