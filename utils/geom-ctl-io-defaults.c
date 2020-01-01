/* for inclusion into geom-ctl-io.c ... this is somewhat of a hack,
 necessitated because gen-ctl-io doesn't write default values (from
 geom.scm) for the input variable, instead relying on them being
 initialized from Scheme. */

integer dimensions = 3;
void *default_material = NULL;
vector3 geometry_center = {0, 0, 0};
lattice geometry_lattice = {{1, 0, 0},
                            {0, 1, 0},
                            {0, 0, 1},
                            {1e20, 1e20, 1e20},
                            {1, 1, 1},
                            {1, 0, 0},
                            {0, 1, 0},
                            {0, 0, 1},
                            {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
                            {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
geometric_object_list geometry = {0, 0};
boolean ensure_periodicity = 0;
