#include "drivers/initrd/tarinitrd.h"

namespace Filesystem {

    // By resolving a path, we add directories to the directory map and add files to the file map.
    void TarReader::resolve_path(char* substring, uint64_t fileInodeNum, TarHeader* currentHeader)
    {
        // We start with the root directory node.
        uint64_t workingDirectory = 0;

        while (substring[0])
        {
            bool isDirectory = false;
            uint64_t substringIndex = 0;
            // Check if the substring contains a directory.
            for (char* pointer = substring; *pointer!='\0'; pointer++)
            {
                if (*pointer == '/')
                {
                    isDirectory = true;
                    substringIndex = pointer - substring;
                    break;
                }
            }
            if (isDirectory)
            {
                char directoryName[100];
                // Get directory name
                strncpy(directoryName, substring, substringIndex);
                directoryName[substringIndex] = '\0';
                strcpy(substring, substring + substringIndex + 1);

                Common::DoublyLinkedListIterator<TarDirectoryEntry> it = (&m_directoryMap.find(0)->last)->entries.first();
                // Try and find directory in map.
                for (it=m_directoryMap.find(workingDirectory)->last.entries.first(); !it.is_end(); ++it)
                {
                    if (!strcmp(it->filename, directoryName)) { break; }
                }
                // Add directory to directory map and to parent directory's entries.
                if (it.is_end())
                {
                    m_directoryMap.insert(++m_dirVirtualNodeNum, TarDirectoryDescriptor());
                    auto entry = TarDirectoryEntry();
                    strcpy(entry.filename, directoryName);
                    entry.inodeNum = m_dirVirtualNodeNum;
                    m_directoryMap.find(workingDirectory)->last.entries.append(entry);
                }
                // FIXME: set this to it in the case that the dir already exists
                workingDirectory = m_dirVirtualNodeNum;
            }
            else
            {
                // Add file to directory and to file map.
                if (substring[0] == '\0') { return; }
                auto entry = TarDirectoryEntry();
                entry.inodeNum = fileInodeNum;
                strcpy(entry.filename, substring);
                m_directoryMap.find(workingDirectory)->last.entries.append(entry);
                m_fileMap.insert(fileInodeNum, currentHeader);
                substring[0] = '\0';
            }
        }
    }
