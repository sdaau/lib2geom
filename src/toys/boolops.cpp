#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework.cpp"
#include "transforms.h"
#include "sbasis-geometric.h"

#include <cstdlib>

using namespace Geom;

double rand_d() { return rand() % 100 / 100.0; }
void cairo_region(cairo_t *cr, Region const &r) {
    cairo_set_source_rgba(cr, rand_d(), rand_d(), rand_d(), .75);
    double d = 5.;
    if(!r.fill()) cairo_set_dash(cr, &d, 1, 0);
    cairo_path(cr, r.boundary());
    cairo_stroke(cr);
    cairo_set_dash(cr, &d, 0, 0);
}

void cairo_regions(cairo_t *cr, Regions const &p) {
    srand(0); 
    for(Regions::const_iterator j = p.begin(); j != p.end(); j++)
        cairo_region(cr, *j);
}

void cairo_shape(cairo_t *cr, Shape const &s) {
    cairo_region(cr, s.getOuter()); cairo_regions(cr, s.getInners());
}

void cairo_shapes(cairo_t *cr, Shapes const &s) {
    for(Shapes::const_iterator j = s.begin(); j != s.end(); j++)
        cairo_shape(cr, *j);
}

std::vector<Path> desanitize(Shape const & s) {
    std::vector<Path> ret;
    ret.push_back(s.getOuter().boundary());
    Regions inners = s.getInners();
    for(Regions::const_iterator j = inners.begin(); j != inners.end(); j++)
        ret.push_back(j->boundary());
    return ret;
}

std::vector<Path> desanitize(Shapes const & s) {
    std::vector<Path> ret;
    for(Shapes::const_iterator j = s.begin(); j != s.end(); j++) {
        std::vector<Path> res = desanitize(*j);
        ret.insert(ret.end(), res.begin(), res.end());
    }
    return ret;
}


void mark_crossings(cairo_t *cr, Regions const &a, Regions const &b) {
    std::vector<Crossings> cc = crossings_between(a, b);
    for(unsigned j = 0; j < cc.size(); j++) {
        Crossings c = cc[j];
        for(Crossings::iterator i = c.begin(); i != c.end(); i++) {
            draw_cross(cr, a[i->a].boundary().pointAt(i->ta));
            draw_text(cr, a[i->a].boundary().pointAt(i->ta), i->dir ? "T" : "F");
        }
    }
}
/*
void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

double rand_d() { return rand() % 100 / 100.0; }
void draw_bounds(cairo_t *cr, Path p) {
    srand(0); 
    for(Path::iterator it = p.begin(); it != p.end(); it++) {
        Rect bounds = it->boundsFast();
        cairo_set_source_rgba(cr, rand_d(), rand_d(), rand_d(), .5);
        draw_rect(cr, bounds.min(), bounds.max());
        cairo_stroke(cr);
    }
}

*/

Shape cleanup(std::vector<Path> const &ps) {
    Regions rs = regions_from_paths(ps);

    unsigned ix = outer_index(rs);
    if(ix == rs.size()) ix = 0;

    Regions inners;
    Region outer;
    for(unsigned i = 0; i < rs.size(); i++) {
        if(i == ix) {
            outer = !rs[i].fill() ? rs[i].inverse() : rs[i];
        } else {
            inners.push_back(rs[i].fill() ? rs[i].inverse() : rs[i]);
        }
    }
    
    Piecewise<D2<SBasis> > pw = outer.boundary().toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    
    return Shape(outer, inners) * Geom::Translate(-centre);
}

class BoolOps: public Toy {
    Region a, b;
    unsigned mode;
    Shape as, bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(handles[0]);
        Shape bst = bs * t;
        Region bt = Region(b.boundary() * t, b.fill());
        //cairo_shape(cr, as);
        //cairo_shape(cr, bst);
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 0., 0., 0., 1);
        //mark_crossings(cr, a, bt);
        cairo_stroke(cr);
        //draw_bounds(cr, a);
        //draw_bounds(cr, b);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        
        //Shapes suni = shape_subtract(as, bst); //path_union(a, b);
        //cairo_shapes(cr, suni);
        

        
        Regions x, y, insies = as.inverse().getInners();
        x.push_back(a.inverse()); x.insert(x.end(), insies.begin(), insies.end()); y.push_back(bt.inverse());
        mark_crossings(cr, x, y);
        Regions r = regions_boolean(true, x, y);
        cairo_regions(cr, r);
        
        /*
        Shapes s;
        switch(mode) {
        case 0:
            s = shape_union(as, bst);
            break;
        case 1:
            s = shape_subtract(as, bst);
            break;
        case 2:
            s = shape_intersect(as, bst);
            break;
        case 3:
            s = shape_exclude(as, bst);
            break;
        }
        if(mode<3) cairo_shapes(cr, s); else cairo_path(cr, desanitize(s));
        cairo_fill(cr);
        */
        
        //used to check if it's right
        //for(int i = 0; i < cont.size(); i++) {
        //    if(path_direction(cont[i].boundary()) != cont[i].fill()) std::cout << "wrong!\n";
        //}
        
        /*Shapes uni = path_union(a, bt);
        cairo_set_source_rgba(cr, 1., 0., 0., .5);
        cairo_shapes(cr, uni);
        */
        /*Shapes sub = path_subtract(a, bt);
        cairo_set_source_rgba(cr, 0., 0., 0., .5);
        cairo_shapes(cr, sub);
        cairo_stroke(cr);
        
        std::vector<Path> inte = path_intersect(a, bt);
        cairo_set_source_rgba(cr, 0., 1., 0., .5);
        cairo_paths(cr, inte);
        cairo_stroke(cr);
        */
        //std::cout.rdbuf(cout_buffer);

        //*notify << "Red = Union exterior, Blue = Holes in union\n Green = Intersection\nSubtraction is meant to be shifted.\n";

        *notify << "Operation: " << (mode ? (mode == 1 ? "subtract" : (mode == 2 ? "intersect" : "exclude")) : "union");
        *notify << "\nKeys:\n u = Union   s = Subtract   i = intersect   e = exclude";
        
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'u') mode = 0; else
        if(e->keyval == 's') mode = 1; else
        if(e->keyval == 'i') mode = 2; else
        if(e->keyval == 'e') mode = 3;
        redraw();
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        char *path_a_name="winding.svgd";
        char *path_b_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        std::vector<Path> paths_b = read_svgd(path_b_name);
        
        //paths_b[0] = paths_b[0] * Geom::Scale(Point(.75, .75));
        mode = 0;
        handles.push_back(Point(200,200));
              
        //Paths holes = bs.getHoles();
        //holes.push_back(bs.getOuter() * Geom::Scale(.5, .5));
        
        //bs = Shape(bs.getOuter(), holes);
        Geom::Matrix m = Geom::Translate(Point(300, 300));
        as = cleanup(paths_a) * Geom::Translate(Point(300, 300));
        bs = cleanup(paths_b);
        //bs = shape_subtract(bs, bs * Scale(.5, .5)).front();
        a = as.getOuter();
        b = bs.getOuter();
        //std::cout << "hi\n";
    }
    int should_draw_bounds() {return 0;}
};

int main(int argc, char **argv) {
    init(argc, argv, new BoolOps());
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
