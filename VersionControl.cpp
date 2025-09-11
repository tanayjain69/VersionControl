#include<iostream>
#include<ctime>
#include<algorithm>
#include<vector>
using namespace std;

// Constants
const int SIZE = 1e3;

// Nodes for the Tree Class
class TreeNode{
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

// Custom hashmap that works for both integers and strings
template <typename T1, typename T2>
class HashMap {
private:
    int hash_fn(int key) {
        return key%SIZE;
    }

    int hash_fn(string key) {
        int x=0;
        for (char c:key) x+= (c-'\0');
        return x%SIZE;
    }

public:
    vector<T1> keys;
    vector<T2> nodes;
    T1 init_val;

    HashMap(T1 initial_val) {
        this->keys = vector<T1>(SIZE, initial_val);
        this->nodes = vector<T2>(SIZE, nullptr);
        init_val=initial_val;
    }

    void insert(T1 key, T2 val) {
        int hashed = hash_fn(key);
        if (keys[hashed] == init_val) {
            keys[hashed] = key;
            nodes[hashed] = val;
        } else {
            // Full round
            int tph = hashed;
            tph = (tph+1)%SIZE;
            while ((tph!=hashed) && keys[tph]!=init_val) {
                tph = (tph+1)%SIZE;
            }
            if (tph==hashed) cout<<"HASHMAP FULL"<<'\n';
            else {
                keys[tph] = key;
                nodes[tph] = val;
            }
        }
    }

    T2 get(T1 key) {
        int hashed = hash_fn(key);
        int temp = hashed;
        while (keys[hashed]!=key) {hashed = (hashed+1)%SIZE; if(hashed==temp) break;}
        if (keys[hashed]==key) return nodes[hashed];
        else return nullptr;
    }

    bool delete_entry(T1 key) {
        int hashed = hash_fn(key);
        int temp = hashed;
        while (keys[hashed]!=key) {
            hashed = (hashed+1)%SIZE; 
            if(hashed==temp) break;
        }
        if (keys[hashed]==key) {
            keys[hashed] = init_val; 
            delete nodes[hashed]; 
            return true;
        }
        else return false;
    }
};

// Tree Class to store all versions and snapshots of a file
class Tree {
public:
    TreeNode* root;
    TreeNode* Active;
    HashMap<int, TreeNode*> version_ctrl{-1};
    int total_versions;
    string file_name;
    vector<pair<time_t, int>> snapshotted_versions;
    time_t last_modified;

    Tree (string filename) {
        file_name = filename;
        TreeNode* file =  new TreeNode(0, "", "created file");
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
            TreeNode* newnode = new TreeNode(total_versions, Active->content+content, "new version created");
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            version_ctrl.insert(total_versions, newnode);
            total_versions++;

        } else {
            Active->content += content; 
            Active->last_modified=time(nullptr);
        }
        last_modified = Active->last_modified;
    }

    void update(string content) {
        if (Active->snapshot_taken) {
            TreeNode* newnode  = new TreeNode(total_versions, content, "new version created");
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            version_ctrl.insert(total_versions, newnode);
            total_versions++;

        } else {
            Active->content = content; 
            Active->last_modified=time(nullptr);
        }
        last_modified = Active->last_modified;
    }

    bool rollback(int v_id) {
        if (v_id==-1) {
            if (Active->Parent) {
                Active = Active->Parent; 
                return true;
            } else return false;
        }
        else {
            TreeNode* u = version_ctrl.get(v_id);
            if (u==nullptr) {cout<<"No version with this version id"<<'\n'<<'\n'; return false;}
            else {
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
        for (int i=snapshotted_versions.size()-1; i>=0; i--) {
            TreeNode* v = version_ctrl.get(snapshotted_versions[i].second);
            cout<<"File "<<i+1<<'\n';
            cout<<"File Version ID: "<<v->version_id<<'\n';
            cout<<"Snapshot Timestamp: "<<ctime(&(v->snapshot_timestamp));
            cout<<"Snapshot message: "<<v->message<<'\n';
            cout<<'\n';
        }
    }
};

// MaxHeap for tracking system wide analytics
class MaxHeap {
    vector<int> heap;
    vector<Tree*> v;

    void heapifyup(int idx) {
        while ((idx>0) && (heap[(idx-1)/2]<heap[idx])) {
            swap(heap[idx], heap[(idx - 1) / 2]);
            swap(v[idx], v[(idx - 1) / 2]);
            idx=(idx-1)/2;
        }
    }

    void heapifydown(int idx) {
        int n = heap.size();
        while (true) {
            int l = 2*idx+1, r=2*idx+2, c=idx;
            if ((l<n) && (heap[l]>heap[c])) c=l;
            if ((r<n) && (heap[r]>heap[c])) c=r;
            if (c == idx) break;
            swap(heap[idx], heap[c]);
            swap(v[idx], v[c]);
            idx=c;
        }
    }

public:
    void insert(int val, Tree* file) {
        heap.push_back(val);
        v.push_back(file);
        heapifyup(heap.size()-1);
    }

    // Specifically made to build heap in O(n) time
    void buildHeap(const vector<Tree*>& elements, bool byVersions = false) {
        heap.clear();
        v.clear();
        for (Tree* t : elements) {
            if (byVersions)
                heap.push_back(t->total_versions);
            else
                heap.push_back((int)t->last_modified);
            v.push_back(t);
        }
        // bottom-up heapify (minizes operations)
        for (int i = (heap.size()/2) - 1; i >= 0; i--) {
            heapifydown(i);
        }
    }

    int getMax() {
        if (heap.empty()) cout<<"Empty!!"<<'\n';
        return heap[0];
    }

    Tree* removeTop() {
        if (heap.empty()) return nullptr;
        Tree* maxVal = v[0];
        v[0] = v.back();
        heap[0] = heap.back();
        heap.pop_back();
        v.pop_back();
        if (!heap.empty()) heapifydown(0);
        return maxVal;
    }
};


// Mainloop
int main() {
    vector<Tree*> ALL_FILES;
    HashMap<string, Tree*> FileMap{""};
    MaxHeap recent_edits;
    MaxHeap most_versions;
    string inp;

    // Command loop
    while (true) {
        getline(cin, inp);

        vector<string> v;
        string a;

        // Break input command words
        for (char c:inp) {
            if (c==' ' && v.size()<2) {
                v.push_back(a);
                a="";
            } else a+=c;
        }
        v.push_back(a);

        // Create a new file
        if (v[0] == "CREATE") {
            if (v.size()>2) {
                cout<<"Spaces are not allowed in the filename"<<'\n'<<'\n'; 
                continue;
            }
            Tree* newfile = new Tree(v[1]);
            ALL_FILES.push_back(newfile);
            FileMap.insert(v[1], newfile);
            cout<<"New file "<<v[1]<<" created."<<'\n'<<'\n';
        }

        // Read a file
        else if (v[0] == "READ") {
            Tree* q = FileMap.get(v[1]);
            if (q) {
                cout<<"Here are the contents of the Active version of the file "<<v[1]<<'\n';
                cout<<q->Active->content<<'\n'<<'\n';
            }
            else cout<<"Invalid filename"<<'\n'<<'\n';
        }

        // See all the snapshotted versions of a file
        else if (v[0] == "HISTORY") {
            if (v.size() == 2) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    cout<<"Here are all the snapshotted versions of the file "<<v[1]<<'\n'<<'\n';
                    q->history();
                }
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }

        // Insert content in a file (append)
        else if (v[0] == "INSERT") {
            if (v.size()==3) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    q->insert(v[2]); 
                    cout<<"Content successfully inserted in the "<<v[1]<<" file."<<'\n'<<'\n';
                }
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }

        // Update the content in the file
        else if (v[0] == "UPDATE") {
            if (v.size()==3) {
                Tree* q = FileMap.get(v[1]);
                if (q) {
                    q->update(v[2]); 
                    cout<<"Content for "<<v[1]<<" successfully updated."<<'\n'<<'\n';
                }
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }

        // Snapshot the Active version of the file
        else if (v[0] == "SNAPSHOT") {
            if (v.size()==3) {
                Tree* q = FileMap.get(v[1]);
                if (q) q->snapshot(v[2]);
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }
        
        // Rollback to a particular version of a file
        else if (v[0] == "ROLLBACK") {
            if (v.size()==2 || v.size()==3) {
                Tree* k = FileMap.get(v[1]);
                if (k->rollback(((v.size()==3)?stoi(v[2]):-1))) cout<<"Successfully rolled back to version ID "<<k->Active->version_id<<" for the file "<<v[1]<<'\n'<<'\n';
                else cout<<"Invalid filename or version-id"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }
        
        //File deletion
        else if (v[0] == "DELETE") {
            if (v.size()==2) { 
                if (FileMap.delete_entry(v[1])) {
                    for (int i=0; i<((int)ALL_FILES.size()); i++) if(ALL_FILES[i]->file_name == v[1]) ALL_FILES.erase(ALL_FILES.begin()+i);
                    cout<<"File deleted"<<'\n'<<'\n';
                }
                else cout<<"Invalid filename"<<'\n'<<'\n';
            } else {cout<<"Invalid Command"<<'\n'<<'\n'; continue;}
        }

        // Check the recently edited files
        else if (v[0] == "RECENT_FILES") {
            recent_edits.buildHeap(ALL_FILES, false);
            cout<<"Latest Modified files- "<<'\n';
            int num_files = stoi(v[1]);
            for (int i=0; i<min((int)ALL_FILES.size(), num_files); i++) {
                Tree* m = recent_edits.removeTop();
                if (m) cout<<m->file_name<<'\n'<<'\n';
                else break;
            }
        }

        // Checked the files with the maximum number of versions
        else if (v[0] == "BIGGEST_TREES") {
            most_versions.buildHeap(ALL_FILES, true);
            cout<<"Largest Tree files-"<<'\n';
            int num_files = stoi(v[1]);
            for (int i=0; i<num_files; i++) {
                Tree* m = most_versions.removeTop();
                if (m) cout<<m->file_name<<'\n'<<'\n';
                else break;
            }
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
            cout<<"HISTORY <filename>"<<'\n';
            cout<<"DELETE <filename>"<<'\n';
            cout<<"RECENT_FILES <number>"<<'\n';
            cout<<"BIGGEST_TREES <number>"<<'\n';
            cout<<"STOP"<<'\n';
            cout<<"HELP"<<'\n'<<'\n';
        }

        // End command
        else if (v[0] =="STOP") {

            // Remove all the files to unclog the dynamically allotted memory
            for (int i=0; i<(int)(ALL_FILES.size()); i++) {
                delete ALL_FILES[i];
            }
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
