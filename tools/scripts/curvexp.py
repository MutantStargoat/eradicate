import bpy

def export(fname):
    if len(bpy.context.selected_objects) != 1 or bpy.context.selected_objects[0].type != 'CURVE':
        print("Make sure a single curve is selected")
        return False
    
    curve_obj = bpy.context.selected_objects[0]
    curve = curve_obj.data
    print("Exporting curve: " + curve_obj.name)
    
    try:
        file = open(fname, "w")
    except Exception:
        print("Failed to open file:" + fname)
        return False
        
    file.write("bezcurve {\n")
    
    cpoints = curve.splines[0].bezier_points
    file.write("\t# number of control points: %d\n" % len(cpoints))
    for cp in cpoints:
        file.write("\tcp {\n")
        file.write("\t\tpos = [%f, %f, %f]\n" % (cp.co[0], cp.co[2], -cp.co[1]))
        file.write("\t\tt0 = [%f, %f, %f]\n" % (cp.handle_left[0], cp.handle_left[2], -cp.handle_left[1]))
        file.write("\t\tt1 = [%f, %f, %f]\n" % (cp.handle_right[0], cp.handle_right[2], -cp.handle_right[1]))
        file.write("\t}\n")
    file.write("}\n")
    file.close()
    return True

export("/tmp/foo.curve")
