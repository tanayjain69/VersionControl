#include <iostream>
#include <ctime>
#include <algorithm>
#include <vector>

using namespace std;

// Constants
const int SIZE = 1e3;

class Tree;

// Node for the Tree Class
class TreeNode {
public:
    int version_id;
    string content;
    string message;
    time_t created_timestamp;
    time_t snapshot_timestamp;
    time_t last_modified;
    TreeNode* Parent;
    vector<TreeNode*> children;
    bool snapshot_taken;

    TreeNode(int v_id, string c, string m) {
        version_id = v_id;
        content = c;
        message = m;
        created_timestamp = time(nullptr);
        snapshot_timestamp = 0;
        Parent = nullptr;
        snapshot_taken = false;
        last_modified = created_timestamp;
    }

    void snapshot(string m) {
        snapshot_timestamp = time(nullptr);
        message = m;
        snapshot_taken = true;
    }
};


template <typename T1, typename T2>
class HashMap {
private:
    vector<vector<pair<T1, T2>>> table;

    int hash_fn(int key) {
        return key % SIZE;
    }

    int hash_fn(const string& key) {
        int x = 0;
        for (char c : key) x += c;
        return x % SIZE;
    }

public:
    HashMap() {
        table.resize(SIZE);
    }

    ~HashMap() {}

    void insert(T1 key, T2 val) {
        int index = hash_fn(key);

        for (auto& pair : table[index]) {
            if (pair.first == key) {
                pair.second = val;
                return;
            }
        }
        table[index].push_back({key, val});
    }

    T2 get(T1 key) {
        int index = hash_fn(key);
        for (const auto& pair : table[index]) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        return nullptr;
    }

    bool delete_entry(T1 key) {
        int index = hash_fn(key);
        for (auto it = table[index].begin(); it != table[index].end(); ++it) {
            if (it->first == key) {
                table[index].erase(it);
                return true;
            }
        }
        return false;
    }
};

// Tree Class to store all versions and snapshots of a file
class Tree {
public:
    TreeNode* root;
    TreeNode* Active;
    HashMap<int, TreeNode*> version_ctrl;
    int total_versions;
    string file_name;
    vector<pair<time_t, int>> snapshotted_versions;
    time_t last_modified;

    Tree(string filename) {
        file_name = filename;
        TreeNode* file = new TreeNode(0, "", "created file");
        root = file;
        Active = file;
        file->snapshot("Initial Snapshot");
        snapshotted_versions.push_back({file->snapshot_timestamp, file->version_id});
        version_ctrl.insert(file->version_id, file);
        total_versions = 1;
        last_modified = file->snapshot_timestamp;
    }

    ~Tree() {
        DeleteSubtree(root);
    }

    void DeleteSubtree(TreeNode* node) {
        if (!node) return;
        for (TreeNode* child : node->children) {
            this->DeleteSubtree(child);
        }
        delete node;
    }

    void insert(string content) {
        if (Active->snapshot_taken) {
            TreeNode* newnode = new TreeNode(total_versions, Active->content + content, "new version created");
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            version_ctrl.insert(total_versions, newnode);
            total_versions++;
        } else {
            Active->content += content;
            Active->last_modified = time(nullptr);
        }
        last_modified = Active->last_modified;
    }

    void update(string content) {
        if (Active->snapshot_taken) {
            TreeNode* newnode = new TreeNode(total_versions, content, "new version created");
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            version_ctrl.insert(total_versions, newnode);
            total_versions++;
        } else {
            Active->content = content;
            Active->last_modified = time(nullptr);
        }
        last_modified = Active->last_modified;
    }

    bool rollback(int v_id) {
        if (v_id == -1) {
            if (Active->Parent) {
                Active = Active->Parent;
                return true;
            } else return false;
        } else {
            TreeNode* u = version_ctrl.get(v_id);
            if (u == nullptr) {
                cout<<"No version with this version id"<<'\n'<<'\n';
                return false;
            } else {
                Active = u;
                return true;
            }
        }
    }

    bool snapshot(string m) {
        if (Active->snapshot_taken) {
            cout<<"Snapshot already taken of this file version"<<'\n'<<'\n';
            return false;
        } else {
            Active->last_modified = time(nullptr);
            Active->snapshot(m);
            cout<<"Snapshot taken for the file "<<file_name<<" version id "<<Active->version_id<<'\n'<<'\n';
            snapshotted_versions.push_back({Active->snapshot_timestamp, Active->version_id});
            last_modified = Active->last_modified;
            return true;
        }
    }

    void read() {
        cout<<Active->content<<'\n'<<'\n';
    }

    void history() {
        sort(snapshotted_versions.begin(), snapshotted_versions.end());
        for (int i = snapshotted_versions.size() - 1; i >= 0; i--) {
            TreeNode* v = version_ctrl.get(snapshotted_versions[i].second);
            cout<<"File "<<i + 1<<'\n';
            cout<<"File Version ID: "<<v->version_id<<'\n';
            cout<<"Snapshot Timestamp: "<<ctime(&(v->snapshot_timestamp));
            cout<<"Snapshot message: "<<v->message<<'\n';
            cout<<'\n';
        }
    }
};


class MaxHeap {
private:
    vector<int> heap;
    vector<Tree*> v; 

    void heapifyup(int idx) {
        while (idx > 0 && heap[(idx - 1) / 2] < heap[idx]) {
            swap(heap[idx], heap[(idx - 1) / 2]);
            swap(v[idx], v[(idx - 1) / 2]);
            idx = (idx - 1) / 2;
        }
    }

    void heapifydown(int idx) {
        int n = heap.size();
        while (true) {
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;
            int largest = idx;

            if (left < n && heap[left] > heap[largest]) largest = left;
            if (right < n && heap[right] > heap[largest]) largest = right;

            if (largest == idx) break;

            swap(heap[idx], heap[largest]);
            swap(v[idx], v[largest]);
            idx = largest;
        }
    }

public:
    void insert(int val, Tree* file) {
        heap.push_back(val);
        v.push_back(file);
        heapifyup(heap.size() - 1);
    }
    
    void update(Tree* file, int new_priority) {
        int index_to_update = -1;
        for (int i = 0; i < v.size(); ++i) {
            if (v[i] == file) {
                index_to_update = i;
                break;
            }
        }

        if (index_to_update != -1) {
            int old_priority = heap[index_to_update];
            heap[index_to_update] = new_priority;

            if (new_priority > old_priority) {
                heapifyup(index_to_update);
            } else {
                heapifydown(index_to_update);
            }
        }
    }

    int getMax() {
        if (heap.empty()) {
            cout<<"Empty!!"<<'\n';
            return -1;
        }
        return heap[0];
    }

    Tree* removeTop() {
        if (heap.empty()) return nullptr;
        Tree* maxVal = v[0];
        v[0] = v.back();
        heap[0] = heap.back();
        heap.pop_back();
        v.pop_back();

        if (!heap.empty()) {
            heapifydown(0);
        }
        return maxVal;
    }
};


// Mainloop
int main() {
    vector<Tree*> ALL_FILES;
    HashMap<string, Tree*> FileMap;
    MaxHeap recent_edits;
    MaxHeap most_versions;
    string inp;

    // Command loop
    while (true) {
        getline(cin, inp);

        vector<string> v;
        string a;

        // Break input command words
        for (char c : inp) {
            if (c == ' ' && v.size() < 2) {
                v.push_back(a);
                a = "";
            } else a += c;
        }
        v.push_back(a);

        // Create a new file
        if (v[0] == "CREATE") {
            if (v.size() > 2) {
                cout<<"Spaces are not allowed in the filename"<<'\n'<<'\n';
                continue;
            }
            Tree* q = FileMap.get(v[1]);
            if (!q) {
                Tree* newfile = new Tree(v[1]);
                ALL_FILES.push_back(newfile);
                FileMap.insert(v[1], newfile);
                
                // Add the new file to the heaps
                recent_edits.insert((int)newfile->last_modified, newfile);
                most_versions.insert(newfile->total_versions, newfile);
                
                cout<<"New file "<<v[1]<<" created."<<'\n'<<'\n';
            } else cout<<"File with this name already exists, try another name"<<'\n'<<'\n';
        }

        // Read a file
        else if (v[0] == "READ") {
            Tree* q = FileMap.get(v[1]);
            if (q) {
                cout<<"Here are the contents of the Active version of the file "<<v[1]<<'\n';
                cout<<q->Active->content<<'\n'<<'\n';
            } else cout<<"Invalid filename"<<'\n'<<'\n';
        }

        // See all the snapshotted versions of a file
        else if (v[0] == "HISTORY") {
            if (v.size() == 2) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    cout<<"Here are all the snapshotted versions of the file "<<v[1]<<'\n'<<'\n';
                    q->history();
                } else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        // Insert content in a file (append)
        else if (v[0] == "INSERT") {
            if (v.size() == 3) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    q->insert(v[2]);
                    // Update heaps after modification
                    recent_edits.update(q, (int)q->last_modified);
                    most_versions.update(q, q->total_versions);
                    cout<<"Content successfully inserted in the "<<v[1]<<" file."<<'\n'<<'\n';
                } else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        // Update the content in the file
        else if (v[0] == "UPDATE") {
            if (v.size() == 3) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    q->update(v[2]);

                    // Update heaps after modification
                    recent_edits.update(q, (int)q->last_modified);
                    most_versions.update(q, q->total_versions);
                    cout<<"Content for "<<v[1]<<" successfully updated."<<'\n'<<'\n';
                } else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        // Snapshot the Active version of the file
        else if (v[0] == "SNAPSHOT") {
            if (v.size() == 3) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    if(q->snapshot(v[2])) {
                         recent_edits.update(q, (int)q->last_modified);
                    }
                }
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        // Rollback to a particular version of a file
        else if (v[0] == "ROLLBACK") {
            if (v.size() == 2 || v.size() == 3) {
                Tree* k = FileMap.get(v[1]);
                if (k && k->rollback(((v.size() == 3) ? stoi(v[2]) : -1))) {
                     cout<<"Successfully rolled back to version ID "<<k->Active->version_id<<" for the file "<<v[1]<<'\n'<<'\n';
                }
                else cout<<"Invalid filename or version-id"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        //File deletion
        else if (v[0] == "DELETE") {
            if (v.size() == 2) {
                if (FileMap.delete_entry(v[1])) {
                    for (int i = 0; i < ((int)ALL_FILES.size()); i++) {
                        if (ALL_FILES[i]->file_name == v[1]) {
                             delete ALL_FILES[i];
                             ALL_FILES.erase(ALL_FILES.begin() + i);
                             break;
                        }
                    }
                    cout<<"File deleted"<<'\n'<<'\n';
                } else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {
                cout<<"Invalid Command"<<'\n'<<'\n';
                continue;
            }
        }

        // Check the recently edited files
        else if (v[0] == "RECENT_FILES") {
            MaxHeap temp_heap = recent_edits;
            cout<<"Latest Modified files- "<<'\n';
            int num_files = stoi(v[1]);
            for (int i = 0; i < min((int)ALL_FILES.size(), num_files); i++) {
                Tree* m = temp_heap.removeTop();
                if (m) cout<<m->file_name<<'\n';
                else break;
            }
             cout<<'\n';
        }

        // Checked the files with the maximum number of versions
        else if (v[0] == "BIGGEST_TREES") {
            MaxHeap temp_heap = most_versions;
            cout<<"Largest Tree files-"<<'\n';
            int num_files = stoi(v[1]);
            for (int i = 0; i < min((int)ALL_FILES.size(), num_files); i++) {
                Tree* m = temp_heap.removeTop();
                if (m) cout<<m->file_name<<'\n';
                else break;
            }
             cout<<'\n';
        }

        // Type help to see the available commands with their syntaxes
        else if (v[0] == "HELP") {
            cout<<"Following are the available commands with their corresponding syntax"<<'\n';
            cout<<"READ <filename>"<<'\n';
            cout<<"CREATE <filename>"<<'\n';
            cout<<"INSERT <filename> <content>"<<'\n';
            cout<<"UPDATE <filename> <content>"<<'\n';
            cout<<"SNAPSHOT <filename> <message>"<<'\n';
            cout<<"ROLLBACK <filename> <version-id>"<<'\n';
            cout<<"ROLLBACK <filename> (to previous version)"<<'\n';
            cout<<"HISTORY <filename>"<<'\n';
            cout<<"DELETE <filename>"<<'\n';
            cout<<"RECENT_FILES <number>"<<'\n';
            cout<<"BIGGEST_TREES <number>"<<'\n';
            cout<<"STOP"<<'\n';
            cout<<"HELP"<<'\n'<<'\n';
        }

        // End command
        else if (v[0] == "STOP") {
            ALL_FILES.clear();
            break;
        }

        // Invalid command check
        else {
            cout<<"INVALID COMMAND. Please Try Again."<<'\n';
            cout<<"You can type 'HELP' to see all the available commands"<<'\n'<<'\n';
        }
    }

    return 0;
}
