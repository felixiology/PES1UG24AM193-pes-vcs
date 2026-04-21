int commit_create(const char *message, ObjectID *commit_id_out) {
    // 1. Create a "Tree" from the Index
    // This turns your staged files into a permanent snapshot in the object store.
    ObjectID tree_id;
    if (tree_from_index(&tree_id) != 0) {
        fprintf(stderr, "Error: Could not create tree from index.\n");
        return -1;
    }

    // 2. Initialize our Commit structure
    Commit c;
    memset(&c, 0, sizeof(c));
    c.tree = tree_id;

    // 3. Find the Parent
    // If head_read succeeds, it means there's a previous commit to link to.
    // If it fails, this is the very first commit (the "root" commit).
    if (head_read(&c.parent) == 0) {
        c.has_parent = 1;
    } else {
        c.has_parent = 0;
    }

    // 4. Add Metadata (Who, When, and Why)
    // pes_author() is a helper that gets your name/SRN from the environment.
    snprintf(c.author, sizeof(c.author), "%s", pes_author());
    c.timestamp = (uint64_t)time(NULL);
    snprintf(c.message, sizeof(c.message), "%s", message);

    // 5. Turn the Commit struct into a text buffer
    // This formats it into the "tree hash / parent hash / author / message" layout.
    void *commit_data;
    size_t commit_len;
    if (commit_serialize(&c, &commit_data, &commit_len) != 0) {
        return -1;
    }

    // 6. Write the Commit object to the Warehouse (.pes/objects)
    if (object_write(OBJ_COMMIT, commit_data, commit_len, commit_id_out) != 0) {
        free(commit_data);
        return -1;
    }

    // 7. Update the Branch Pointer
    // Move 'main' (or HEAD) to point to this brand new commit.
    if (head_update(commit_id_out) != 0) {
        free(commit_data);
        return -1;
    }

    // Clean up the memory we allocated
    free(commit_data);
    return 0;
}
//////
