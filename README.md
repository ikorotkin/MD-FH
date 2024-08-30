# Modified GROMACS for writing full trajectory every time step

Unlike the trr-files natively supported by GROMACS, this extension writes a series of smaller files adding `000000`, `000001`, `000002`, etc. suffixes to the output file name. This allows the user to post-process MD trajectories "on the fly", during the simulation, and delete the output files after post-processing to save disk space.

Another major difference is that in addition to the full trajectory, this extension writes the atom masses at the beginning of each file.

The total output file size generated by the extension is approximately the same as a single trr-file produced by GROMACS.

This GROMACS extension comes with a header-only [C++ reader](https://github.com/ikorotkin/MD-FH/tree/master/traj_reader) that reads the output files into memory for further post-processing by the user. Here is an [example cpp-file](https://github.com/ikorotkin/MD-FH/blob/master/traj_reader/read_traj_example.cpp) of how to use the reader.

## How to modify GROMACS

Tested on GROMACS 2023.1.

### Step 1.

Locate `src/gromacs/mdrun/md.cpp` file and open it for editing.

### Step 2.

Add the following code block below all `#include` directives (approximately after line 155):

```cpp
// Modified Gromacs - code block 1/2

#include <fstream>

const std::string out_file_name = "traj";     // Output file name without file extension
const std::string out_file_name_ext = "out";  // Output file extension (e.g., "dat")

const int N_out_frames_per_file = 1000;  // Number of frames to write into each out-file

int N_out_frame_counter = 0;  // Counts `write_out_frame` function calls

std::ofstream out_file;  // Output file stream

std::string out_file_name_to_close;  // Previous output file name (used to add extension)

/*
 * Writes variable `var` to the file stream
 */
template <typename T> inline void write_data_to_out(T var)
{
    out_file.write(reinterpret_cast<char*>(&var), sizeof(var));
}

/*
 * Correctly closes the output file stream and renames the output file adding the extension
 */
int out_file_close()
{
    if(out_file && out_file.is_open())
    {
        out_file.close();

        // Full file name with extension
        std::string new_file_name = out_file_name_to_close + "." + out_file_name_ext;

        // Add file extension
        if(std::rename(out_file_name_to_close.c_str(), new_file_name.c_str()) != 0)
        {
            return 0;  // Error renaming file
        }
    }

    return -1;
}

/*
 * Writes entire frame (full trajectory at the given time step) to the file stream
 */
int write_out_frame(int64_t step,
                    real t,
                    const rvec* box,
                    int natoms,
                    const rvec* x,
                    const rvec* v,
                    const rvec* f,
                    const std::vector<real> &mass,
                    bool last_step)
{
    // Initial output
    if(!step)
    {
        printf("\n==== MODIFIED GROMACS -- Writes full trajectories every time step! ====\n\n");
    }

    // Should we create a new file or not
    bool new_file = !static_cast<bool>(N_out_frame_counter % N_out_frames_per_file);

    if(new_file)
    {
        // Close and rename the output file if needed
        if(!out_file_close())
        {
            return 0;  // Error renaming the file
        }

        // New file name suffix
        int file_suffix = N_out_frame_counter / N_out_frames_per_file;

        // Convert the number to a string
        std::string num_str = std::to_string(file_suffix);

        // Add leading zeros if necessary
        std::string num_str_zeros = std::string(6 - num_str.length(), '0') + num_str;

        // Full file name (without extension)
        std::string fname = out_file_name + "." + num_str_zeros;

        // Save the file name for renaming purposes later
        out_file_name_to_close = fname;

        // Open binary file for writing
        out_file.open(fname, std::ios::binary);

        // Check if the file is opened successfully
        if(out_file && out_file.is_open())
        {
            // All good
        }
        else
        {
            return 0;  // Error opening file
        }
    }

    // Write data
    if(out_file.is_open())
    {
        // MD box size
        float Lx = box[0][0];
        float Ly = box[1][1];
        float Lz = box[2][2];
        
        int step_int = static_cast<int>(step);  // Time step - narrowing for writing
        
        // Header
        write_data_to_out(step_int);  // Current time step (int)
        write_data_to_out(natoms);    // Number of atoms (int)
        write_data_to_out(t);         // Current time (float)
        write_data_to_out(Lx);        // Box size Lx (float)
        write_data_to_out(Ly);        // Box size Ly (float)
        write_data_to_out(Lz);        // Box size Lz (float)

        // Frame
        for(int n = 0; n < natoms; n++)
        {
            write_data_to_out(mass[n]);  // Atom mass (float)

            for(int d = 0; d < 3; d++)
            {
                write_data_to_out(x[n][d]);  // Coordinates (float)
                write_data_to_out(v[n][d]);  // Velocities (float)
                // write_data_to_out(f[n][d]);  // Forces (float)
            }
        }
    }
    else
    {
        return 0;
    }

    N_out_frame_counter++;

    // Close and rename the output file if needed
    if(last_step)
    {
        if(!out_file_close())
        {
            return 0;  // Error renaming the file
        }
    }

    return -1;
}
```

### Step 3.

Search for `upd.update_coords` function call in `md.cpp` and add the following code block **above** (approximately before line 1770):

```cpp
// Modified Gromacs - code block 2/2

/*
 * Custom output to a binary file
 */
if (MAIN(cr))
{
    if (!write_out_frame(step,
                         t,
                         const_cast<rvec*>(state->box),
                         top_global.natoms,
                         const_cast<rvec*>(state->x.rvec_array()),
                         const_cast<rvec*>(state->v.rvec_array()),
                         as_rvec_array(forceCombined.unpaddedConstArrayRef().data()),
                         md->massT,
                         bLastStep && step_rel == ir->nsteps))
    {
        gmx_file("Cannot write trajectory frame to the out-file; maybe you are out of disk space?");
    }
}
```

### Step 4.

Rebuild modified GROMACS.

Here is an [example](https://github.com/ikorotkin/MD-FH/blob/master/gromacs_modified/md.cpp) of modified `md.cpp` file for GROMACS 2023.1.
