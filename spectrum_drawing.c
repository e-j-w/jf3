int getFirstNonemptySpectrum(int numSpOpened){
  if(numSpOpened>=NSPECT){
    return -1;
  }
  int i,j;
  for(i=0;i<numSpOpened;i++){
    for(j=0;j<S32K;j++){
      if(rawdata.hist[i][j]!=0.0){
        return i;
      }
    }
  }
  return -1;
}

//get the value of the ith bin of the displayed spectrum
//i is in channel units, offset from drawing.lowerLimit
//for contracted spectra, the original channel units are retained, but the sum
//of j bins is returned, where j is the contraction factor
//spNum is the displayed spectrum number (for multiplot), 0 is the first displayed spectrum
float getDispSpBinVal(int dispSpNum, int bin){

  if((dispSpNum >= drawing.numMultiplotSp)||(dispSpNum < 0)){
    //invalid displayed spectrum number
    return 0;
  }

  int j,k;
  float val = 0.;
    for(j=0;j<drawing.contractFactor;j++){
      switch(drawing.multiplotMode){
        case 1:
          //sum spectra
          for(k=0;k<drawing.numMultiplotSp;k++){
            val += rawdata.hist[drawing.multiPlots[k]][drawing.lowerLimit+bin+j];
          }
          break;
        case 4:
          //stacked
        case 3:
          //overlay (independent scaling)
        case 2:
          //overlay (common scaling)
        case 0:
          //no multiplot
          val += rawdata.hist[drawing.multiPlots[dispSpNum]][drawing.lowerLimit+bin+j];
          break;
        default:
          break;
      }
    }
  
  return val;
}

//function setting the plotting limits for the spectrum based on the zoom level
//the plotting limits are in UNCALIBRATED units ie. channels
void getPlotLimits(){
    if(drawing.zoomLevel <= 1.0){
        //set zoomed out
        drawing.zoomLevel = 1.0;
        drawing.lowerLimit = 0;
        drawing.upperLimit = S32K - 1;
        return;
    }else if(drawing.zoomLevel > 1024.0){
        drawing.zoomLevel = 1024.0; //set maximum zoom level
    }

    int numChansToDisp = (int)(1.0*S32K/drawing.zoomLevel);
    drawing.lowerLimit = drawing.xChanFocus - numChansToDisp/2;
    drawing.lowerLimit = drawing.lowerLimit - (drawing.lowerLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
    //clamp to lower limit of 0 if needed
    if(drawing.lowerLimit < 0){
        drawing.lowerLimit = 0;
        drawing.upperLimit = numChansToDisp - 1;
        return;
    }
    drawing.upperLimit = drawing.xChanFocus + numChansToDisp/2;
    drawing.upperLimit = drawing.upperLimit - (drawing.upperLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
    //clamp to upper limit of S32K-1 if needed
    if(drawing.upperLimit > (S32K-1)){
        drawing.upperLimit=S32K-1;
        drawing.lowerLimit=S32K-1-numChansToDisp;
        return;
    }
}

void on_toggle_autoscale(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    drawing.autoScale=1;
  else
    drawing.autoScale=0;
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
}

void on_toggle_cursor(GtkToggleButton *togglebutton, gpointer user_data)
{
  if(gtk_toggle_button_get_active(togglebutton))
    gui.drawSpCursor=0;
  else
    gui.drawSpCursor=-1;
  gtk_widget_queue_draw(GTK_WIDGET(cursor_drawing_area));
}


//function handling mouse wheel scrolling to zoom the displayed spectrum
void on_spectrum_scroll(GtkWidget *widget, GdkEventScroll *e)
{
  if(!rawdata.openedSp){
		return;
	}

  if(e->x < 80.0){
    //out of plot range
    return;
  }

  if(e->direction == 1){
    //printf("Scrolling down at %f %f!\n",e->x,e->y);
    drawing.zoomLevel *= 0.5; 
  }else if(drawing.zoomLevel < 1024.0){
    //printf("Scrolling up at %f %f!\n",e->x,e->y);
    drawing.zoomLevel *= 2.0;
    GdkRectangle dasize;  // GtkDrawingArea size
    GdkWindow *wwindow = gtk_widget_get_window(widget);
    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry (wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
    drawing.xChanFocus = drawing.lowerLimit + (((e->x)-80.0)/(dasize.width-80.0))*(drawing.upperLimit - drawing.lowerLimit);
  }
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
}

void on_spectrum_cursor_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data){

  if(!rawdata.openedSp){
		return;
	}

  //printf("Cursor pos: %f %f\n",event->x,event->y);
  if (event->state & GDK_BUTTON1_MASK){
    //button being pressed
    if(gui.draggingSp == 0){
      //start drag
      gui.draggingSp = 1;
      if(gui.drawSpCursor == 1)
        gui.drawSpCursor = 0; //hide vertical cursor while dragging
      gui.dragstartul=drawing.upperLimit;
      gui.dragstartll=drawing.lowerLimit;
      gui.dragStartX = event->x;
      //printf("Drag started! dragstartll=%i, dragstartul=%i\n",gui.dragstartll,gui.dragstartul);
    }else{
      //continue drag
      //printf("Drag updated!\n");
      GdkRectangle dasize;  // GtkDrawingArea size
      GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
      // Determine GtkDrawingArea dimensions
      gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
      int limitWidth = drawing.upperLimit-drawing.lowerLimit;
      drawing.upperLimit = gui.dragstartul - ((2.*(event->x - gui.dragStartX)/(dasize.width))*limitWidth);
      drawing.lowerLimit = gui.dragstartll - ((2.*(event->x - gui.dragStartX)/(dasize.width))*limitWidth);
      drawing.xChanFocus = drawing.lowerLimit + (drawing.upperLimit - drawing.lowerLimit)/2.;
      //printf("startx = %f, x = %f, drawing.lowerLimit = %i, drawing.upperLimit = %i, width = %i, focus = %i\n",gui.dragStartX,event->x,drawing.lowerLimit,drawing.upperLimit,dasize.width,drawing.xChanFocus);
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }else{
    //no button press
    gui.draggingSp = 0;
  }

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((event->x > 80.0)&&(event->y < (dasize.height - 40.0))){
    
    int cursorChan = drawing.lowerLimit + (((event->x)-80.0)/(dasize.width-80.0))*(drawing.upperLimit - drawing.lowerLimit);
    cursorChan = cursorChan - fmod(cursorChan,drawing.contractFactor);

    //print cursor position on status bar
    char posLabel[256];
    char *posLabelp = posLabel;
    int i;
    switch(drawing.multiplotMode){
      case 4:
      case 3:
      case 2:
        //multiple visible plots
        if(calpar.calMode == 1){
          int cursorChanEnd = cursorChan + drawing.contractFactor;
          float cal_lowerChanLimit = calpar.calpar0 + calpar.calpar1*cursorChan + calpar.calpar2*cursorChan*cursorChan;
          float cal_upperChanLimit = calpar.calpar0 + calpar.calpar1*cursorChanEnd + calpar.calpar2*cursorChanEnd*cursorChanEnd;
          posLabelp += snprintf(posLabelp,50,"%s: %0.1f - %0.1f, Values:", calpar.calUnit, cal_lowerChanLimit, cal_upperChanLimit);
        }else{
          if(drawing.contractFactor <= 1){
            posLabelp += snprintf(posLabel,50,"Channel: %i, Values:",cursorChan);
          }else{
            snprintf(posLabel,256,"Channels: %i - %i, Values:",cursorChan, cursorChan + drawing.contractFactor - 1);
          }
        }
        for(i=0;i<(drawing.numMultiplotSp-1);i++){
          posLabelp += snprintf(posLabelp,17," %0.1f,", getDispSpBinVal(i,cursorChan-drawing.lowerLimit));
        }
        posLabelp += snprintf(posLabelp,17," %0.1f", getDispSpBinVal(drawing.numMultiplotSp-1,cursorChan-drawing.lowerLimit));
        break;
      case 1:
      case 0:
      default:
        //single plot
        if(calpar.calMode == 1){
          int cursorChanEnd = cursorChan + drawing.contractFactor;
          float cal_lowerChanLimit = calpar.calpar0 + calpar.calpar1*cursorChan + calpar.calpar2*cursorChan*cursorChan;
          float cal_upperChanLimit = calpar.calpar0 + calpar.calpar1*cursorChanEnd + calpar.calpar2*cursorChanEnd*cursorChanEnd;
          snprintf(posLabel,256,"%s: %0.1f - %0.1f, Value: %0.1f", calpar.calUnit, cal_lowerChanLimit, cal_upperChanLimit, getDispSpBinVal(0,cursorChan-drawing.lowerLimit));
        }else{
          if(drawing.contractFactor <= 1){
            snprintf(posLabel,256,"Channel: %i, Value: %0.1f",cursorChan,getDispSpBinVal(0,cursorChan-drawing.lowerLimit));
          }else{
            snprintf(posLabel,256,"Channels: %i - %i, Value: %0.1f",cursorChan, cursorChan + drawing.contractFactor - 1, getDispSpBinVal(0,cursorChan-drawing.lowerLimit));
          }
        }
        break;
    }
    gtk_label_set_text(bottom_info_text,posLabel);

    //draw cursor on plot (expensive, requires redraw of plot itself)
    if((gui.draggingSp == 0)&&(gui.drawSpCursor != -1)){
      //don't redraw if the cursor hasn't moved, that would be st00pid
      if(fabs(gui.cursorPosX - event->x) >= 1.0){
        gui.cursorPosX = event->x;
        gui.drawSpCursor = 1; //draw vertical cursor
        gtk_widget_queue_draw(GTK_WIDGET(cursor_drawing_area));
      }
    }

  }else{
    gtk_label_set_text(bottom_info_text,"");
    if(gui.drawSpCursor == 1){
      gui.drawSpCursor = 0; //hide vertical cursor
      gtk_widget_queue_draw(GTK_WIDGET(cursor_drawing_area));
    }
  }

  return;
}

//get the bin position in the histogram plot
float getXPos(int bin, float clip_x1, float clip_x2){
	return clip_x1 + 80.0 + (bin*(clip_x2-clip_x1-80.0)/(drawing.upperLimit-drawing.lowerLimit));
}

float getYPos(float val, int multiplotSpNum, float clip_y1, float clip_y2){
  float pos;
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      pos = clip_y1 + 40.0 + (clip_y2-clip_y1-40.0)*((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])));
      if(pos < clip_y1 + 40.0 + (clip_y2-clip_y1-40.0)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)))
        pos = clip_y1 + 40.0 + (clip_y2-clip_y1-40.0)*(multiplotSpNum/(drawing.numMultiplotSp*1.0));
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      //single plot
      pos = clip_y1 + 40.0 + (clip_y2-clip_y1-40.0)*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]));
      if(pos < clip_y1 + 40.0)
        pos = clip_y1 + 40.0;
      break;
  }
	return pos;
}

//axis tick drawing
float getAxisXPos(int axisVal, float clip_x1, float clip_x2){
  int cal_lowerLimit = drawing.lowerLimit;
  int cal_upperLimit = drawing.upperLimit;
  if(calpar.calMode==1){
    //calibrate
    cal_lowerLimit = calpar.calpar0 + calpar.calpar1*drawing.lowerLimit + calpar.calpar2*drawing.lowerLimit*drawing.lowerLimit;
    cal_upperLimit = calpar.calpar0 + calpar.calpar1*drawing.upperLimit + calpar.calpar2*drawing.upperLimit*drawing.upperLimit;
  }
  if((axisVal < cal_lowerLimit)||(axisVal >= cal_upperLimit))
    return -1; //value is off the visible axis
  
  return clip_x1 + 80.0 + (clip_x2-clip_x1-80.0)*(axisVal - cal_lowerLimit)/(cal_upperLimit - cal_lowerLimit);
}
void drawXAxisTick(int axisVal, cairo_t *cr, float clip_x1, float clip_x2, float clip_y1, float clip_y2, double baseFontSize){
  int axisPos = getAxisXPos(axisVal,clip_x1,clip_x2);
  if(axisPos >= 0){
    //axis position is valid
    cairo_move_to (cr, axisPos, -40.0-clip_y1);
    cairo_line_to (cr, axisPos, -35.0-clip_y1);
    char tickLabel[20];
    sprintf(tickLabel,"%i",axisVal); //set string for label
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, axisPos - extents.width/2., -20.0);
    cairo_show_text(cr, tickLabel);
  }
}
float getAxisYPos(float axisVal, int multiplotSpNum, float clip_y2){
  float posVal;

  switch(drawing.multiplotMode){
    case 4:
      //stacked
      posVal = (1.0/drawing.numMultiplotSp)*(40.0-clip_y2)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) + (40.0-clip_y2)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - 40.0;
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      posVal = (40.0-clip_y2)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) - 40.0;
      break;
  }

  //printf("clip_y2: %f, multiplotsp: %i, axisval: %f, posval: %f\n",clip_y2,multiplotSpNum,axisVal,posVal);
  return posVal;
}
void drawYAxisTick(float axisVal, int multiplotSpNum, cairo_t *cr, float clip_x1, float clip_x2, float clip_y1, float clip_y2, double baseFontSize){
  if((axisVal < drawing.scaleLevelMin[multiplotSpNum])||(axisVal >= drawing.scaleLevelMax[multiplotSpNum])){
    //printf("axisval:%f,scalemin:%f,scalemax:%f\n",axisVal,drawing.scaleLevelMin[multiplotSpNum],drawing.scaleLevelMax[multiplotSpNum]);
    return; //invalid axis value,
  }
  if((axisVal!=0.0)&&(fabs(axisVal - 0) < (drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/20.0)){
    //tick is too close to zero, don't draw
    return;
  }
  float axisPos = getAxisYPos(axisVal,multiplotSpNum,clip_y2);
  //printf ("clip: %f %f\n",clip_y1,clip_y2);
  if((axisPos <= 0) && (axisPos > (clip_y2-clip_y1)*-0.98)) {
    //axis position is valid (ie. on the plot, and not too close to the top of the plot so that it won't be cut off)
    cairo_move_to (cr, clip_x1 + 85.0, axisPos);
    cairo_line_to (cr, clip_x1 + 75.0, axisPos);
    char tickLabel[20];
    if((axisVal>=1000.0)||(axisVal<=-1000.0)){
      sprintf(tickLabel,"%0.1e",axisVal); //set string for label
    }else{
      sprintf(tickLabel,"%0.1f",axisVal); //set string for label
    }
    
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, clip_x1 + 70.0 - extents.width, axisPos + extents.height/2.);
    cairo_show_text(cr, tickLabel);
  }
}

void drawPlotLabel(cairo_t *cr, float clip_x1, float clip_x2, float clip_y2, double baseFontSize){
  char plotLabel[256];
  int i;
  cairo_text_extents_t extents; //get dimensions needed to justify text labels
  cairo_set_font_size(cr, baseFontSize);
  switch(drawing.multiplotMode){
    case 4:
      //stacked spectra
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (clip_x2-clip_x1)*0.95 - extents.width, (40.0-clip_y2)*((i/(drawing.numMultiplotSp*1.0)) + 1.0/drawing.numMultiplotSp) - 20.0);
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 3:
    case 2:
      //overlaid spectra
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (clip_x2-clip_x1)*0.95 - extents.width, (40.0-clip_y2)*0.9 - 40.0 + 18.0*i);
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 1:
      //summed spectra
      cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
      strcpy(plotLabel, "Sum of:");
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (clip_x2-clip_x1)*0.95 - extents.width, (40.0-clip_y2)*0.9 - 40.0);
      cairo_show_text(cr, plotLabel);
      for(i=0;i<drawing.numMultiplotSp;i++){
        strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (clip_x2-clip_x1)*0.95 - extents.width, (40.0-clip_y2)*0.9 - 40.0 + 18.0*(i+1));
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 0:
      //single plot mode
      cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
      strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[0]]);
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (clip_x2-clip_x1)*0.95 - extents.width, (40.0-clip_y2)*0.9 - 40.0);
      cairo_show_text(cr, plotLabel);
      break;
    default:
      break;
  }
}

//get the x range of the plot in terms of x axis units, 
//taking into account whether or not a calibration is in use
int getPlotRangeXUnits(){
  int cal_lowerLimit = drawing.lowerLimit;
  int cal_upperLimit = drawing.upperLimit;
  if(calpar.calMode==1){
    //calibrate
    cal_lowerLimit = calpar.calpar0 + calpar.calpar1*drawing.lowerLimit + calpar.calpar2*drawing.lowerLimit*drawing.lowerLimit;
    cal_upperLimit = calpar.calpar0 + calpar.calpar1*drawing.upperLimit + calpar.calpar2*drawing.upperLimit*drawing.upperLimit;
  }
  return cal_upperLimit - cal_lowerLimit;
}

//draw a cursor over the spectrum
void drawCursorArea(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{

	if(!rawdata.openedSp){
		return;
	}

  if(gui.drawSpCursor != 1){
    return;
  }

  //printf("Drawing cursor!\n");

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *wwindow = gtk_widget_get_window(widget);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  cairo_move_to(cr, gui.cursorPosX, 0.0);
  cairo_line_to(cr, gui.cursorPosX, dasize.height-40.0);
  cairo_stroke(cr);

}

//draw a spectrum
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{

	if(!rawdata.openedSp){
		return;
	}

  //printf("Drawing spectrum!\n");

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *wwindow = gtk_widget_get_window(widget);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);

	if (drawing.multiPlots[0] >= NSPECT)
	{
		printf("Spectrum number too high (%i)!\n", drawing.multiPlots[0]);
		return;
	}

  int i,j;
	
  // Draw the background colour
  //cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  //cairo_paint(cr);

  // transform the coordinate system
  cairo_translate(cr, 0.0, dasize.height); //so that the origin is at the lower left
  cairo_scale(cr, 1.0, -1.0); //so that positive y values go up

  // Determine the data points to calculate (ie. those in the clipping zone
  gdouble clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
  cairo_clip_extents(cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
  cairo_set_line_width(cr, 2.0);

  double plotFontSize = 13.5;

  getPlotLimits(); //setup the x range to plot over

  //get the maximum/minimum y values of the displayed region
  float maxVal[MAX_DISP_SP];
  float minVal[MAX_DISP_SP];
  for(i=0;i<drawing.numMultiplotSp;i++){
    minVal[i] = BIG_NUMBER;
    maxVal[i] = SMALL_NUMBER;
  }
  for(i=0;i<(drawing.upperLimit-drawing.lowerLimit-1);i+=drawing.contractFactor){
    float currentVal[MAX_DISP_SP];
    switch(drawing.multiplotMode){
      case 4:
        //stacked
      case 3:
        //overlay (independent scaling)
        for(j=0;j<drawing.numMultiplotSp;j++){
          currentVal[j] = getDispSpBinVal(j, i);
          if(currentVal[j] > maxVal[j]){
            maxVal[j] = currentVal[j];
          }
          if(currentVal[j] < minVal[j]){
            minVal[j] = currentVal[j];
          }
        }
        break;
      case 2:
        //overlay (common scaling)
        for(j=0;j<drawing.numMultiplotSp;j++){
          currentVal[0] = getDispSpBinVal(j, i);
          if(currentVal[0] > maxVal[0]){
            maxVal[0] = currentVal[0];
          }
          if(currentVal[0] < minVal[0]){
            minVal[0] = currentVal[0];
          }
        }
        break;
      case 1:
        //summed
      case 0:
        currentVal[0] = getDispSpBinVal(0, i);
        if(currentVal[0] > maxVal[0]){
          maxVal[0] = currentVal[0];
        }
        if(currentVal[0] < minVal[0]){
          minVal[0] = currentVal[0];
        }
        break;
      default:
        break;
    }
  }
  //setup autoscaling
  if((drawing.autoScale)||(drawing.scaleLevelMax[0] <= drawing.scaleLevelMin[0])){
    switch(drawing.multiplotMode){
      case 4:
      case 3:
        for(i=0;i<drawing.numMultiplotSp;i++){
          drawing.scaleLevelMax[i] = maxVal[i]*1.1;
          drawing.scaleLevelMin[i] = minVal[i];
        }
        break;
      case 2:
      case 1:
      case 0:
        drawing.scaleLevelMax[0] = maxVal[0]*1.1;
        drawing.scaleLevelMin[0] = minVal[0];
        break;
      default:
        break;
    }
  }
  //printf("maxVal = %f, minVal = %f\n",maxVal[0],minVal[0]);

  //interpolate (ie. limit the number of bins drawn) in the next step, 
  //to help drawing performance
  int maxDrawBins = (clip_x2 - clip_x1)*2;
  //printf("maximum bins to draw: %i\n",maxDrawBins);
  int binSkipFactor = (drawing.upperLimit-drawing.lowerLimit)/(maxDrawBins);
  if(binSkipFactor <= drawing.contractFactor)
    binSkipFactor = drawing.contractFactor;
  //printf("binskipfactor: %i\n",binSkipFactor);
  //for smooth scrolling of interpolated spectra, have the start bin always
  //be a multiple of the skip factor
  int startBin = 0 - (drawing.lowerLimit % binSkipFactor);

	//draw the actual histogram
  switch(drawing.multiplotMode){
    case 4:
      //stacked
    case 3:
      //overlay (independent scaling)
      for(i=0;i<drawing.numMultiplotSp;i++){
        for(j=startBin;j<(drawing.upperLimit-drawing.lowerLimit-1);j+=binSkipFactor){
          float currentVal = getDispSpBinVal(i, j);
          float nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_move_to (cr, getXPos(j,clip_x1,clip_x2), getYPos(currentVal,i,clip_y1,clip_y2));
          cairo_line_to (cr, getXPos(j+binSkipFactor,clip_x1,clip_x2), getYPos(currentVal,i,clip_y1,clip_y2));
          cairo_line_to (cr, getXPos(j+binSkipFactor,clip_x1,clip_x2), getYPos(nextVal,i,clip_y1,clip_y2));
        }
        //choose color
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case 2:
      //overlay (common scaling)
      for(i=0;i<drawing.numMultiplotSp;i++){
        for(j=startBin;j<(drawing.upperLimit-drawing.lowerLimit-1);j+=binSkipFactor){
          float currentVal = getDispSpBinVal(i, j);
          float nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_move_to (cr, getXPos(j,clip_x1,clip_x2), getYPos(currentVal,0,clip_y1,clip_y2));
          cairo_line_to (cr, getXPos(j+binSkipFactor,clip_x1,clip_x2), getYPos(currentVal,0,clip_y1,clip_y2));
          cairo_line_to (cr, getXPos(j+binSkipFactor,clip_x1,clip_x2), getYPos(nextVal,0,clip_y1,clip_y2));
        }
        //choose color
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case 1:
      //summed
    case 0:
      for(i=startBin;i<(drawing.upperLimit-drawing.lowerLimit-1);i+=binSkipFactor){
        float currentVal = getDispSpBinVal(0, i);
        float nextVal = getDispSpBinVal(0, i+binSkipFactor);
        //printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,clip_x1,clip_x2), rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],clip_x1,clip_x2);
        cairo_move_to (cr, getXPos(i,clip_x1,clip_x2), getYPos(currentVal,0,clip_y1,clip_y2));
        cairo_line_to (cr, getXPos(i+binSkipFactor,clip_x1,clip_x2), getYPos(currentVal,0,clip_y1,clip_y2));
        cairo_line_to (cr, getXPos(i+binSkipFactor,clip_x1,clip_x2), getYPos(nextVal,0,clip_y1,clip_y2));
      }
      cairo_set_source_rgb (cr, drawing.spColors[0], drawing.spColors[1], drawing.spColors[2]);
      cairo_stroke(cr);
      break;
    default:
      break;
  }

  // draw axis lines
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  cairo_move_to (cr, clip_x1+80.0, clip_y1+40.0);
  cairo_line_to (cr, clip_x1+80.0, clip_y2);
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_move_to (cr, clip_x1+80.0, clip_y1+40.0 + (clip_y2-clip_y1-40.0)*i/(drawing.numMultiplotSp*1.0));
        cairo_line_to (cr, clip_x2, clip_y1+40.0 + (clip_y2-clip_y1-40.0)*i/(drawing.numMultiplotSp*1.0));
      }
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      //single plot
      cairo_move_to (cr, clip_x1+80.0, clip_y1+40.0);
      cairo_line_to (cr, clip_x2, clip_y1+40.0);
      break;
  }
  cairo_stroke(cr);
  

  
  cairo_scale(cr, 1.0, -1.0); //remove axis inversion, so that text is the right way up

  //draw x axis ticks
  if(getPlotRangeXUnits() > 20000){
    for(i=0;i<S32K;i+=5000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 10000){
    for(i=0;i<S32K;i+=2000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 5000){
    for(i=0;i<S32K;i+=1000){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 3000){
    for(i=0;i<S32K;i+=500){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 2000){
    for(i=0;i<S32K;i+=300){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 1000){
    for(i=0;i<S32K;i+=200){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 500){
    for(i=0;i<S32K;i+=100){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 200){
    for(i=0;i<S32K;i+=50){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 100){
    for(i=0;i<S32K;i+=20){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else if(getPlotRangeXUnits() > 50){
    for(i=0;i<S32K;i+=10){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }else{
    for(i=0;i<S32K;i+=5){
      drawXAxisTick(i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
    }
  }
  cairo_stroke(cr);
  
  //draw y axis ticks
  int numTickPerSp;
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      numTickPerSp = (clip_y2 - clip_y1)/(40.0*drawing.numMultiplotSp) + 1;
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        for(j=0;j<numTickPerSp;j++){
          drawYAxisTick(drawing.scaleLevelMin[i] + (drawing.scaleLevelMax[i] - drawing.scaleLevelMin[i])*j/(numTickPerSp*1.), i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
        }
        drawYAxisTick(0.0, i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize); //always draw the zero label on the y axis
        cairo_stroke(cr);
        //draw the zero line if applicable
        if((drawing.scaleLevelMin[i] < 0.0) && (drawing.scaleLevelMax[i] > 0.0)){
          cairo_set_line_width(cr, 1.0);
          cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
          cairo_move_to (cr, clip_x1 + 80.0, getAxisYPos(0.0,i,clip_y2));
          cairo_line_to (cr, clip_x2, getAxisYPos(0.0,i,clip_y2));
          cairo_stroke(cr);
        }
      }
      break;
    case 3:
      //overlay (independent scaling)
      for(i=0;i<drawing.numMultiplotSp;i++){
        float labelOffset = 0.4*(i+1)/(drawing.numMultiplotSp*1.);
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        drawYAxisTick(drawing.scaleLevelMax[i]*(0.3 + labelOffset), i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize); //draw one axis tick near the middle of the axis, per spectrum
        drawYAxisTick(0.0, i, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize); //always draw the zero label on the y axis
      }
      break;
    case 2:
    case 1:
    case 0:
      //modes with a single scale
      cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
      numTickPerSp = (clip_y2 - clip_y1)/40.0 + 1;
      for(i=0;i<numTickPerSp;i++){
        drawYAxisTick(drawing.scaleLevelMin[0] + (drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])*i/(numTickPerSp*1.), 0, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize);
      }
      drawYAxisTick(0.0, 0, cr, clip_x1, clip_x2, clip_y1, clip_y2, plotFontSize); //always draw the zero label on the y axis
      cairo_stroke(cr);
      //draw the zero line if applicable
      if((drawing.scaleLevelMin[0] < 0.0) && (drawing.scaleLevelMax[0] > 0.0)){
        cairo_set_line_width(cr, 1.0);
        cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
        cairo_move_to (cr, clip_x1 + 80.0, getAxisYPos(0.0,0,clip_y2));
        cairo_line_to (cr, clip_x2, getAxisYPos(0.0,0,clip_y2));
        cairo_stroke(cr);
      }
      break;
    default:
      break;
  }

  drawPlotLabel(cr,clip_x1,clip_x2,clip_y2, plotFontSize); //draw plot label(s)
  cairo_stroke(cr);
  

  //draw axis labels
  cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  char axisLabel[16];
  cairo_text_extents_t extents; //for getting dimensions needed to center text labels
  //x axis
  if(calpar.calMode == 0){
    sprintf(axisLabel,"Channel #"); //set string for label
  }else{
    strcpy(axisLabel,calpar.calUnit); //set label to calibrated units
  }
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, (clip_x2-clip_x1)*0.55 - (extents.width/2), -clip_y1-1.0);
  cairo_show_text(cr, axisLabel);
  //y axis
  sprintf(axisLabel,"Value"); //set string for label
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, clip_x1+12.0, (clip_y1-clip_y2)*0.5);
  cairo_save(cr); //store the context before the rotation
  cairo_rotate(cr, 1.5*3.14159);
  cairo_translate(cr, (clip_x2-clip_x1)*0.015, -1.0*((clip_y1-clip_y2)*0.5)); //so that the origin is at the lower left
  cairo_show_text(cr, axisLabel);
  cairo_stroke(cr);
  cairo_restore(cr); //recall the unrotated context


  return;
}
