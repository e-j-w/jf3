/* J. Williams, 2020 */

//This file contains routines for drawing spectra using Cairo.
//The main routine is drawSpectrum (near the bottom), helper 
//subroutines are above it.

//set the default text color, depending on the
void setTextColor(cairo_t *cr){
  if(guiglobals.preferDarkTheme){
    cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
  }else{
    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
  }
  
}

//converts cursor position units to channel units on the displayed spectrum
//return value is float to allow sub-channel prescision, cast it to int if needed
float getCursorChannel(const float cursorx, const float cursory, const float xorigin, const float yorigin){
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > xorigin)&&(cursory < (dasize.height - yorigin))){
    
    float cursorChan = drawing.lowerLimit + (((cursorx)-xorigin)/(dasize.width-xorigin))*(drawing.upperLimit - drawing.lowerLimit);
    //printf("chan: %f\n",cursorChan);
    return cursorChan;
    //return cursorChan - fmod(cursorChan,drawing.contractFactor);
  }
  return -1; //cursor not over spectrum
}

//converts cursor position units to y-value on the displayed spectrum
//this is the value on the displayed y-axis, at the cursor postion
float getCursorYVal(const float cursorx, const float cursory, const float xorigin, const float yorigin){
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > xorigin)&&(cursory < (dasize.height - yorigin))){
    float cursorVal;
    switch (drawing.multiplotMode)
    {
      case 1:
      case 0:
        //single plot mode
        if(drawing.logScale){
          if(drawing.scaleLevelMin[0] > 0){
            cursorVal = pow(10.0,(dasize.height-yorigin - cursory)*log10(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/(dasize.height-yorigin)) + drawing.scaleLevelMin[0];
          }else{
            cursorVal = pow(10.0,(dasize.height-yorigin - cursory)*log10(drawing.scaleLevelMax[0])/(dasize.height-yorigin));
          }
        }else{
          cursorVal = drawing.scaleLevelMax[0] - ((cursory)/(dasize.height-yorigin))*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0]);
          //printf("cursorVal: %f, scaleLevelMin: %f, scaleLevelMax: %f\n",cursorVal,drawing.scaleLevelMin[0],drawing.scaleLevelMax[0]);
        }
        break;
      default:
        return 0; //not implemented
        break;
    }
    //printf("cursory: %f, cursorVal: %f, scaleMax: %f, scaleMin: %f\n",cursory,cursorVal,drawing.scaleLevelMax[0],drawing.scaleLevelMin[0]);
    return cursorVal;
    //return cursorChan - fmod(cursorChan,drawing.contractFactor);
  }
  return 0; //cursor not over spectrum
}

//get the index of the comment at which the cursor is over
//return -1 if no comment is at the cursor position
//some shameless magic numbers used to map channel and y-values to comment indicator size
int getCommentAtCursor(const float cursorx, const float cursory, const float xorigin, const float yorigin){
  int i;
  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
  if((cursorx > xorigin)&&(cursory < (dasize.height - yorigin))){
    float cursorCh = getCursorChannel(cursorx, cursory, xorigin, yorigin);
    float cursorYVal = getCursorYVal(cursorx, cursory, xorigin, yorigin);
    switch (drawing.multiplotMode)
    {
      case 1:
        //sum plot mode
        for(i=0;i<rawdata.numChComments;i++){
          if(rawdata.chanCommentView[i] == 1){
            if(rawdata.chanCommentSp[i] == drawing.displayedView){
              //check proximity to channel
              if(fabs(rawdata.chanCommentCh[i] - cursorCh) < (30.0*(drawing.upperLimit - drawing.lowerLimit)/dasize.width)){
                //check proximity to y-val
                float chYVal = rawdata.chanCommentVal[i];
                if(chYVal < drawing.scaleLevelMin[0]){
                  chYVal = drawing.scaleLevelMin[0];
                }else if(chYVal > drawing.scaleLevelMax[0]){
                  chYVal = drawing.scaleLevelMax[0];
                }
                if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/dasize.height)){
                  return i; //this comment is close
                }
              }
            }
          }
        }
        break;
      case 0:
        //single plot mode
        if(drawing.displayedView == -1){
          for(i=0;i<rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i] == 0){
              if(rawdata.chanCommentSp[i] == drawing.multiPlots[0]){
                //check proximity to channel
                if(fabs(rawdata.chanCommentCh[i] - cursorCh) < (30.0*(drawing.upperLimit - drawing.lowerLimit)/dasize.width)){
                  //check proximity to y-val
                  float chYVal = rawdata.chanCommentVal[i];
                  if(chYVal < drawing.scaleLevelMin[0]){
                    chYVal = drawing.scaleLevelMin[0];
                  }else if(chYVal > drawing.scaleLevelMax[0]){
                    chYVal = drawing.scaleLevelMax[0];
                  }
                  if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/dasize.height)){
                    return i; //this comment is close
                  }
                }
              }
            }
          }
        }else{
          //scaled single spectrum view
          for(i=0;i<rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i] == 1){
              if(rawdata.chanCommentSp[i] == drawing.displayedView){
                //check proximity to channel
                if(fabs(rawdata.chanCommentCh[i] - cursorCh) < (30.0*(drawing.upperLimit - drawing.lowerLimit)/dasize.width)){
                  //check proximity to y-val
                  float chYVal = rawdata.chanCommentVal[i];
                  if(chYVal < drawing.scaleLevelMin[0]){
                    chYVal = drawing.scaleLevelMin[0];
                  }else if(chYVal > drawing.scaleLevelMax[0]){
                    chYVal = drawing.scaleLevelMax[0];
                  }
                  if(fabs(chYVal - cursorYVal) < (30.0*(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0])/dasize.height)){
                    return i; //this comment is close
                  }
                }
              }
            }
          }
        }
        
        break;
      default:
        break;
    }
  }
  return -1; //cursor not over a comment, or comments not implemented for the drawing mode
}


//function setting the plotting limits for the spectrum based on the zoom level
//the plotting limits are in UNCALIBRATED units ie. channels
void setPlotLimits(){
  if(drawing.zoomLevel <= 1.0){
      //set zoomed out
      drawing.zoomLevel = 1.0;
      drawing.lowerLimit = 0;
      drawing.upperLimit = S32K - 1;
      return;
  }else if(drawing.zoomLevel > 1024.0){
      drawing.zoomLevel = 1024.0; //set maximum zoom level
  }

  if(drawing.zoomFocusFrac > 1.0){
    drawing.zoomFocusFrac = 1.0;
  }else if(drawing.zoomFocusFrac < 0.0){
    drawing.zoomFocusFrac = 0.0;
  }

  int numChansToDisp = (int)(1.0*S32K/drawing.zoomLevel);
  drawing.lowerLimit = drawing.xChanFocus - numChansToDisp*drawing.zoomFocusFrac;
  drawing.lowerLimit = drawing.lowerLimit - (drawing.lowerLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
  //clamp to lower limit of 0 if needed
  if(drawing.lowerLimit < 0){
      drawing.lowerLimit = 0;
      drawing.upperLimit = numChansToDisp - 1;
      return;
  }
  drawing.upperLimit = drawing.xChanFocus + numChansToDisp*(1.0 - drawing.zoomFocusFrac);
  drawing.upperLimit = drawing.upperLimit - (drawing.upperLimit % drawing.contractFactor); //round to nearest multiple of contraction factor
  //clamp to upper limit of S32K-1 if needed
  if(drawing.upperLimit > (S32K-1)){
      drawing.upperLimit=S32K-1;
      drawing.lowerLimit=S32K-1-numChansToDisp;
      return;
    }
}

//zoom to the non-zero region of the spectrum
void autoZoom(){
  if(drawing.multiplotMode == 0){
    int i;
    for(i=0;i<S32K;i++){
      if(rawdata.hist[drawing.multiPlots[0]][i] != 0.){
        drawing.lowerLimit = i;
        break;
      }
    }
    for(i=S32K-1;i>=0;i--){
      if(rawdata.hist[drawing.multiPlots[0]][i] != 0.){
        drawing.upperLimit = i;
        break;
      }
    }
    drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2.0;
    drawing.zoomFocusFrac = 0.5;
    int numChansToDisp = drawing.upperLimit - drawing.lowerLimit;
    if(numChansToDisp > 0){
      drawing.zoomLevel = 1.0*S32K/numChansToDisp;
    }

    //obey zoom limits
    if(drawing.zoomLevel < 1.0){
      drawing.zoomLevel = 1.0;
    }else if(drawing.zoomLevel > 1024.0){
      drawing.zoomLevel = 1024.0; //set maximum zoom level
    }
      
  }
  //printf("lowerLimit: %i, upperLimit: %i, xChanFocus: %i, zoomLevel: %f\n",drawing.lowerLimit,drawing.upperLimit,drawing.xChanFocus, drawing.zoomLevel);
}

gboolean zoom_y_callback(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data){
  int i;
  for(i=0;i<drawing.numMultiplotSp;i++){
    if(drawing.scaleLevelMax[i] > drawing.scaleToLevelMax[i]){
      drawing.scaleLevelMax[i] -= 0.2*(drawing.scaleLevelMax[i]-drawing.scaleToLevelMax[i]) + 0.1*fabs(drawing.scaleLevelMax[i]);
      if(drawing.scaleLevelMax[i] <= drawing.scaleToLevelMax[i]){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
      }
    }
    if(drawing.scaleLevelMin[i] < drawing.scaleToLevelMin[i]){
      drawing.scaleLevelMin[i] += 0.2*(drawing.scaleToLevelMin[i]-drawing.scaleLevelMin[i]) + 0.1*fabs(drawing.scaleLevelMin[i]);
      if(drawing.scaleLevelMin[i] >= drawing.scaleToLevelMin[i]){
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }
    if(drawing.scaleLevelMax[i] < drawing.scaleToLevelMax[i]){
      drawing.scaleLevelMax[i] += 0.2*(drawing.scaleToLevelMax[i]-drawing.scaleLevelMax[i]) + 0.1*fabs(drawing.scaleLevelMax[i]);
      if(drawing.scaleLevelMax[i] >= drawing.scaleToLevelMax[i]){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
      }
    }
    if(drawing.scaleLevelMin[i] > drawing.scaleToLevelMin[i]){
      drawing.scaleLevelMin[i] -= 0.2*(drawing.scaleLevelMin[i]-drawing.scaleToLevelMin[i]) + 0.1*fabs(drawing.scaleLevelMin[i]);
      if(drawing.scaleLevelMin[i] <= drawing.scaleToLevelMin[i]){
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }
  }
  for(i=0;i<drawing.numMultiplotSp;i++){
    //printf("scaleMin: %f, scaleToMin: %f,   scaleMax: %f, scaleToMax: %f\n",drawing.scaleLevelMin[i],drawing.scaleToLevelMin[i],drawing.scaleLevelMax[i],drawing.scaleToLevelMax[i]);
    if((drawing.scaleLevelMax[i] != drawing.scaleToLevelMax[i])||(drawing.scaleLevelMin[i] != drawing.scaleToLevelMin[i])){
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
      return G_SOURCE_CONTINUE;
    }
  }
  //printf("Finished y zoom.\n");
  drawing.zoomingY = 0; //finished zooming
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  return G_SOURCE_REMOVE;
}

gboolean zoom_in_tick_callback(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data){
  //printf("Zooming x in\n");
  drawing.zoomLevel *= 1.15;
  if((drawing.zoomLevel > drawing.zoomToLevel)||(drawing.zoomLevel > 1024.0)){
    drawing.zoomLevel = drawing.zoomToLevel;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    return G_SOURCE_REMOVE;
  }
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  return G_SOURCE_CONTINUE;
}

gboolean zoom_out_tick_callback(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data){
  //printf("Zooming x out\n");
  drawing.zoomLevel *= 0.87;
  if((drawing.zoomLevel < drawing.zoomToLevel)||(drawing.zoomLevel < 1.0)){
    drawing.zoomLevel = drawing.zoomToLevel;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    return G_SOURCE_REMOVE;
  }
  gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
  gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  return G_SOURCE_CONTINUE;
}

//zoom in on the spectrum, not following the cursor (for use with keyboard shortcut)
void on_zoom_in_x(){
  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }
  if(guiglobals.useZoomAnimations){
    drawing.zoomToLevel = drawing.zoomLevel * 2.0;
    if(drawing.zoomToLevel > 1024.0)
      drawing.zoomToLevel = 1024.0;
    drawing.zoomFocusFrac = 0.5;
    gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_in_tick_callback, NULL, NULL);
  }else{
    drawing.zoomLevel *= 2.0;
    if(drawing.zoomLevel > 1024.0)
      drawing.zoomLevel = 1024.0;
    drawing.zoomFocusFrac = 0.5;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }
}

void on_zoom_out_x(){
  //handle case where this is called by shortcut, and spectra are not open
  if(rawdata.openedSp == 0){
    return;
  }
  if(guiglobals.useZoomAnimations){
    drawing.zoomToLevel = drawing.zoomLevel * 0.5;
    if(drawing.zoomToLevel < 1.0)
      drawing.zoomToLevel = 1.0;
    drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
    drawing.zoomFocusFrac = 0.5;
    gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_out_tick_callback, NULL, NULL);
  }else{
    drawing.zoomLevel *= 0.5;
    if(drawing.zoomLevel < 1.0)
      drawing.zoomLevel = 1.0;
    drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
    drawing.zoomFocusFrac = 0.5;
    gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
    gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
  }
}

//function handling mouse wheel scrolling to zoom the displayed spectrum
void on_spectrum_scroll(GtkWidget *widget, GdkEventScroll *e){
  if(!rawdata.openedSp){
    return;
  }

  if(e->x < 80.0){
    //out of plot range
    return;
  }

  if((e->direction == 1)&&(drawing.zoomLevel > 1.0)){
    //printf("Scrolling down at %f %f!\n",e->x,e->y);
    on_zoom_out_x();
  }else if((e->direction != 1)&&(drawing.zoomLevel < 1024.0)){
    //handle zooming that follows cursor
    //printf("Scrolling up at %f %f!\n",e->x,e->y);
    GdkRectangle dasize;  // GtkDrawingArea size
    GdkWindow *wwindow = gtk_widget_get_window(widget);
    // Determine GtkDrawingArea dimensions
    gdk_window_get_geometry (wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
    if(guiglobals.useZoomAnimations){
      //check if we are already zooming
      drawing.zoomToLevel = drawing.zoomLevel * 2.0;
      drawing.xChanFocus = drawing.lowerLimit + (((e->x)-80.0)/(dasize.width-80.0))*(drawing.upperLimit - drawing.lowerLimit);
      drawing.zoomFocusFrac = (drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit);
      gtk_widget_add_tick_callback (widget, zoom_in_tick_callback, NULL, NULL);
    }else{
      drawing.xChanFocus = drawing.lowerLimit + (((e->x)-80.0)/(dasize.width-80.0))*(drawing.upperLimit - drawing.lowerLimit);
      drawing.zoomLevel *= 2.0;
      drawing.xChanFocus = drawing.lowerLimit + (((e->x)-80.0)/(dasize.width-80.0))*(drawing.upperLimit - drawing.lowerLimit);
      drawing.zoomFocusFrac = (drawing.xChanFocus - drawing.lowerLimit)/(1.0*drawing.upperLimit - drawing.lowerLimit);
      gtk_range_set_value(GTK_RANGE(zoom_scale),log(drawing.zoomLevel)/log(2.));//base 2 log of zoom
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }
  
}

void on_spectrum_click(GtkWidget *widget, GdkEventButton *event, gpointer data){
  
  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)){
    //right mouse button being pressed
    float cursorChan = getCursorChannel(event->x, event->y, 80.0, 40.0);
    switch(guiglobals.fittingSp){
      case 6:
        //fit being displayed, clear it on right click
        guiglobals.fittingSp = 0;
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
        break;
      case 2:
        //setup peak positions
        if(fitpar.numFitPeaks < MAX_FIT_PK){
          if((cursorChan >= fitpar.fitStartCh)&&(cursorChan <= fitpar.fitEndCh)){
            fitpar.fitPeakInitGuess[fitpar.numFitPeaks] = cursorChan;
            printf("Fitting peak at channel %f\n",fitpar.fitPeakInitGuess[fitpar.numFitPeaks]);
            gtk_widget_set_sensitive(GTK_WIDGET(fit_fit_button),TRUE);
            fitpar.numFitPeaks++;
          }
        }
        if(fitpar.numFitPeaks >= MAX_FIT_PK){
          printf("Maximum number of fit peaks specified.\n");
          fitpar.numFitPeaks = MAX_FIT_PK;
          startGausFit(); //force fit to proceed
        }
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
        break;
      case 1:
        //setup fitting limit
        if(fitpar.fitEndCh < 0){
          if(fitpar.fitStartCh < 0){
            fitpar.fitStartCh = (int)cursorChan;
          }else{
            if(cursorChan > fitpar.fitStartCh){
              fitpar.fitEndCh = (int)cursorChan;
            }else if (cursorChan < fitpar.fitStartCh){
              fitpar.fitEndCh = fitpar.fitStartCh; //swap
              fitpar.fitStartCh = (int)cursorChan;
            }
          }
        }
        //check if both limits have been set
        if((fitpar.fitStartCh >= 0)&&(fitpar.fitEndCh >=0)){
          printf("Fit limits: channel %i through %i\n",fitpar.fitStartCh,fitpar.fitEndCh);
          guiglobals.fittingSp = 2;
          update_gui_fit_state();
        }
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
        break;
      case 0:
      default:
        break;
    }
  }else if((event->type == GDK_DOUBLE_BUTTON_PRESS) && (event->button == 1)){
    //double click
    if(rawdata.openedSp){
      float cursorChan, cursorYVal;
      cursorChan = getCursorChannel(event->x, event->y, 80.0, 40.0);
      cursorYVal = getCursorYVal(event->x, event->y, 80.0, 40.0);
      if(cursorChan >= 0){
        //user has double clicked on the displayed spectrum
        switch (drawing.multiplotMode)
        {
          case 1:
            //summed single plot
            //offer option to create a new summed spectrum and comment on that
            guiglobals.commentEditInd = getCommentAtCursor(event->x, event->y, 80.0, 40.0);
            if((guiglobals.commentEditInd >= 0)&&(guiglobals.commentEditInd < NCHCOM)){
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_entry_set_text(comment_entry, rawdata.chanComment[guiglobals.commentEditInd]);
              gtk_button_set_label(comment_ok_button,"Apply");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),TRUE);
            }else{
              if(rawdata.numChComments >= NCHCOM){
                printf("Cannot add any more comments.\n");
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"The maximum number of comments has been reached.  Older comments must be deleted before more can be added.");
                gtk_dialog_run (GTK_DIALOG (message_dialog));
                gtk_widget_destroy (message_dialog);
                return;
              }
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),FALSE);
              //setup comment data
              rawdata.chanCommentVal[(int)rawdata.numChComments] = cursorYVal;
              rawdata.chanCommentCh[(int)rawdata.numChComments] = cursorChan;
              rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
              if(drawing.displayedView >= 0){
                rawdata.chanCommentSp[(int)rawdata.numChComments] = drawing.displayedView;
                gtk_button_set_label(comment_ok_button,"Apply");
              }else if (drawing.displayedView == -2){
                //this is a view that hasn't been saved yet
                if(rawdata.numViews >= MAXNVIEWS){
                  printf("Cannot add any more views.\n");
                  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                  GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Adding a comment would require the current view to be saved, however the maximum number of custom views has been reached.  Older custom views must be deleted before more can be added.");
                  gtk_dialog_run (GTK_DIALOG (message_dialog));
                  gtk_widget_destroy (message_dialog);
                  return;
                }
                rawdata.chanCommentSp[(int)rawdata.numChComments] = rawdata.numViews;
                gtk_button_set_label(comment_ok_button,"Save View and Apply");
              }else{
                printf("WARNING: undefined view state, not displaying edit window.\n");
              }
              gtk_entry_set_text(comment_entry, "");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }
            //re-enable comment display
            guiglobals.drawSpComments=1;
            //show the window to edit the comment
            gtk_window_present(comment_window);
            break;
          case 0:
            //single plot being displayed
            //open a dialog for the user to write a comment
            //printf("Double click on channel %f, value %f\n",cursorChan,cursorYVal);
            guiglobals.commentEditInd = getCommentAtCursor(event->x, event->y, 80.0, 40.0);
            if((guiglobals.commentEditInd >= 0)&&(guiglobals.commentEditInd < NCHCOM)){
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),TRUE);
              gtk_entry_set_text(comment_entry, rawdata.chanComment[guiglobals.commentEditInd]);
              gtk_button_set_label(comment_ok_button,"Apply");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),TRUE);
            }else{
              if(rawdata.numChComments >= NCHCOM){
                printf("Cannot add any more comments.\n");
                GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"The maximum number of comments has been reached.  Older comments must be deleted before more can be added.");
                gtk_dialog_run (GTK_DIALOG (message_dialog));
                gtk_widget_destroy (message_dialog);
                return;
              }
              gtk_widget_set_sensitive(GTK_WIDGET(remove_comment_button),FALSE);
              //setup comment data
              rawdata.chanCommentVal[(int)rawdata.numChComments] = cursorYVal;
              rawdata.chanCommentCh[(int)rawdata.numChComments] = cursorChan;
              if(drawing.displayedView == -1){
                //commenting on a raw spectrum, not a view
                rawdata.chanCommentSp[(int)rawdata.numChComments] = drawing.multiPlots[0];
                rawdata.chanCommentView[(int)rawdata.numChComments] = 0;
              }else if(drawing.displayedView >= 0){
                //commenting on a saved view
                rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
                rawdata.chanCommentSp[(int)rawdata.numChComments] = drawing.displayedView;
                gtk_button_set_label(comment_ok_button,"Apply");
              }else if (drawing.displayedView == -2){
                //commenting on a view that hasn't been saved yet
                if(rawdata.numViews >= MAXNVIEWS){
                  printf("Cannot add any more views.\n");
                  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
                  GtkWidget *message_dialog = gtk_message_dialog_new(window, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Cannot add comment!");
                  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message_dialog),"Adding a comment would require the current view to be saved, however the maximum number of custom views has been reached.  Older custom views must be deleted before more can be added.");
                  gtk_dialog_run (GTK_DIALOG (message_dialog));
                  gtk_widget_destroy (message_dialog);
                  return;
                }
                rawdata.chanCommentView[(int)rawdata.numChComments] = 1;
                rawdata.chanCommentSp[(int)rawdata.numChComments] = rawdata.numViews;
                gtk_button_set_label(comment_ok_button,"Save View and Apply");
              }else{
                printf("WARNING: undefined view state, not displaying edit window.\n");
              }
              gtk_entry_set_text(comment_entry, "");
              gtk_widget_set_sensitive(GTK_WIDGET(comment_ok_button),FALSE);
            }
            //re-enable comment display
            guiglobals.drawSpComments=1;
            //show the window to edit the comment
            gtk_window_present(comment_window); 
            break;
          default:
            break;
        }
      }
    }
  }
}


void on_spectrum_cursor_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data){

  if(!rawdata.openedSp){
    return;
  }

  //printf("Cursor pos: %f %f\n",event->x,event->y);
  if (event->state & GDK_BUTTON1_MASK){
    //left mouse button being pressed
    if(guiglobals.draggingSp == 0){
      //start drag
      guiglobals.draggingSp = 1;
      if(guiglobals.drawSpCursor == 1)
        guiglobals.drawSpCursor = 0; //hide vertical cursor while dragging
      guiglobals.dragstartul=drawing.upperLimit;
      guiglobals.dragstartll=drawing.lowerLimit;
      guiglobals.dragStartX = event->x;
      drawing.xChanFocus = (drawing.upperLimit + drawing.lowerLimit)/2;
      drawing.zoomFocusFrac = 0.5;
      //printf("Drag started! dragstartll=%i, dragstartul=%i\n",guiglobals.dragstartll,guiglobals.dragstartul);
    }else{
      //continue drag
      //printf("Drag updated!\n");
      GdkRectangle dasize;  // GtkDrawingArea size
      GdkWindow *gwindow = gtk_widget_get_window(spectrum_drawing_area);
      // Determine GtkDrawingArea dimensions
      gdk_window_get_geometry (gwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);
      drawing.xChanFocus = (guiglobals.dragstartul + guiglobals.dragstartll)/2. + ((guiglobals.dragStartX - event->x)/(dasize.width-80.0))*(guiglobals.dragstartul - guiglobals.dragstartll);
      drawing.zoomFocusFrac = 0.5;
      //printf("startx = %f, x = %f, drawing.lowerLimit = %i, drawing.upperLimit = %i, width = %i, focus = %i\n",guiglobals.dragStartX,event->x,drawing.lowerLimit,drawing.upperLimit,dasize.width,drawing.xChanFocus);
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }else{
    //no button press
    guiglobals.draggingSp = 0;
  }

  int cursorChan = (int)getCursorChannel(event->x, event->y, 80.0, 40.0);
  int cursorChanRounded = cursorChan - (cursorChan % drawing.contractFactor); //channel at the start of the bin (if rebinned)
  //printf("cursorChan: %i\n",cursorChan);

  if(cursorChan >= 0){

    int commentToHighlight = getCommentAtCursor(event->x, event->y, 80.0, 40.0);
    if(commentToHighlight != drawing.highlightedComment){
      //highlight the comment
      drawing.highlightedComment = commentToHighlight;
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
    
    int peakToHighlight = -1;
    if(guiglobals.fittingSp == 6){
      //check if the cursor is over a peak
      int i;
      for(i=0;i<fitpar.numFitPeaks;i++){
        if(cursorChan > (fitpar.fitParVal[7+(3*i)] - 2.*fitpar.fitParVal[8+(3*i)])){
          if(cursorChan < (fitpar.fitParVal[7+(3*i)] + 2.*fitpar.fitParVal[8+(3*i)])){
            if(peakToHighlight == -1){
              peakToHighlight = i;
            }else{
              //highlight whichever peak is closer to the cursor
              if(fabs(cursorChan - fitpar.fitParVal[7+(3*i)]) < fabs(cursorChan - fitpar.fitParVal[7+(3*drawing.highlightedPeak)])){
                peakToHighlight = i;
              }
            }
          }
        }
      }
    }

    //set the peak to highlight in the drawing routine
    if(drawing.highlightedPeak != peakToHighlight){
      //highlight the peak
      drawing.highlightedPeak = peakToHighlight;
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }

    //print info on the status bar
    char statusBarLabel[256];
    char *statusBarLabelp = statusBarLabel;
    char binValStr[50];
    //char *binValStrp = binValStr;
    float binVal;
    int i;
    switch(drawing.highlightedPeak){
      case -1:
        //print cursor position on status bar
        switch(drawing.multiplotMode){
          case 4:
          case 3:
          case 2:
            //multiple visible plots
            if(calpar.calMode == 1){
              int cursorChanEnd = cursorChanRounded + drawing.contractFactor;
              float cal_lowerChanLimit = getCalVal(cursorChanRounded);
              float cal_upperChanLimit = getCalVal(cursorChanEnd);
              statusBarLabelp += snprintf(statusBarLabelp,50,"%s: %0.1f - %0.1f, Values:", calpar.calUnit, cal_lowerChanLimit, cal_upperChanLimit);
            }else{
              if(drawing.contractFactor <= 1){
                statusBarLabelp += snprintf(statusBarLabel,50,"Channel: %i, Values:",cursorChanRounded);
              }else{
                statusBarLabelp += snprintf(statusBarLabel,256,"Channels: %i - %i, Values:",cursorChanRounded, cursorChanRounded + drawing.contractFactor - 1);
              }
            }
            for(i=0;i<(drawing.numMultiplotSp-1);i++){
              binVal = getDispSpBinVal(i,cursorChanRounded-drawing.lowerLimit);
              getFormattedValAndUncertainty(binVal,sqrt(fabs(binVal)),binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
              statusBarLabelp += snprintf(statusBarLabelp,17," %s,", binValStr);
            }
            binVal = getDispSpBinVal(drawing.numMultiplotSp-1,cursorChanRounded-drawing.lowerLimit);
            getFormattedValAndUncertainty(binVal,sqrt(fabs(binVal)),binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
            binValStr[15] = '\0'; //truncate string (staying safe with sprintf, working around compiler warning when using snprintf instead)
            statusBarLabelp += sprintf(statusBarLabelp," %s", binValStr);
            break;
          case 1:
          case 0:
          default:
            //single plot
            binVal = getDispSpBinVal(0,cursorChanRounded-drawing.lowerLimit);
            getFormattedValAndUncertainty(binVal,sqrt(fabs(binVal)),binValStr,50,guiglobals.showBinErrors,guiglobals.roundErrors);
            if(calpar.calMode == 1){
              int cursorChanEnd = cursorChanRounded + drawing.contractFactor;
              float cal_lowerChanLimit = getCalVal(cursorChanRounded);
              float cal_upperChanLimit = getCalVal(cursorChanEnd);
              snprintf(statusBarLabel,256,"%s: %0.1f to %0.1f, Value: %s", calpar.calUnit,cal_lowerChanLimit,cal_upperChanLimit,binValStr);
            }else{
              if(drawing.contractFactor <= 1){
                snprintf(statusBarLabel,256,"Channel: %i, Value: %s",cursorChanRounded,binValStr);
              }else{
                snprintf(statusBarLabel,256,"Channels: %i to %i, Value: %s",cursorChanRounded,cursorChanRounded + drawing.contractFactor - 1,binValStr);
              }
            }
            break;
        }
        break;
      default:
        //print highlighted peak info
        if(calpar.calMode == 1){
          float calCentr = getCalVal(fitpar.fitParVal[7+(3*drawing.highlightedPeak)]);
          float calWidth = getCalWidth(fitpar.fitParVal[8+(3*drawing.highlightedPeak)]);
          if(fitpar.errFound){
            float calCentrErr = getCalWidth(fitpar.fitParErr[7+(3*drawing.highlightedPeak)]);
            float calWidthErr = getCalWidth(fitpar.fitParErr[8+(3*drawing.highlightedPeak)]);
            char fitParStr[3][50];
            getFormattedValAndUncertainty(evalPeakArea(drawing.highlightedPeak,fitpar.fitType),evalPeakAreaErr(drawing.highlightedPeak,fitpar.fitType),fitParStr[0],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(calCentr,calCentrErr,fitParStr[1],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(2.35482*calWidth,2.35482*calWidthErr,fitParStr[2],50,1,guiglobals.roundErrors);
            snprintf(statusBarLabel,256,"Area: %s, Centroid: %s, FWHM: %s",fitParStr[0],fitParStr[1],fitParStr[2]);
          }else{
            snprintf(statusBarLabel,256,"Area: %0.3f, Centroid: %0.3f, FWHM: %0.3f",evalPeakArea(drawing.highlightedPeak,fitpar.fitType),calCentr,2.35482*calWidth);
          }
        }else{
          if(fitpar.errFound){
            char fitParStr[3][50];
            getFormattedValAndUncertainty(evalPeakArea(drawing.highlightedPeak,fitpar.fitType),evalPeakAreaErr(drawing.highlightedPeak,fitpar.fitType),fitParStr[0],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(fitpar.fitParVal[7+(3*drawing.highlightedPeak)],fitpar.fitParErr[7+(3*drawing.highlightedPeak)],fitParStr[1],50,1,guiglobals.roundErrors);
            getFormattedValAndUncertainty(2.35482*fitpar.fitParVal[8+(3*drawing.highlightedPeak)],2.35482*fitpar.fitParErr[8+(3*drawing.highlightedPeak)],fitParStr[2],50,1,guiglobals.roundErrors);
            snprintf(statusBarLabel,256,"Area: %s, Centroid: %s, FWHM: %s",fitParStr[0],fitParStr[1],fitParStr[2]);
          }else{
            snprintf(statusBarLabel,256,"Area: %0.3f, Centroid: %0.3Lf, FWHM: %0.3Lf",evalPeakArea(drawing.highlightedPeak,fitpar.fitType),fitpar.fitParVal[7+(3*drawing.highlightedPeak)],2.35482*fitpar.fitParVal[8+(3*drawing.highlightedPeak)]);
          }
        }

        break;
    }
    gtk_label_set_text(bottom_info_text,statusBarLabel);

    //draw cursor on plot (expensive, requires redraw of plot itself)
    if((guiglobals.draggingSp == 0)&&(guiglobals.drawSpCursor != -1)){
      //don't redraw if the cursor hasn't moved, that would be st00pid
      if(fabs(guiglobals.cursorPosX - event->x) >= 1.0){
        guiglobals.cursorPosX = event->x;
        guiglobals.drawSpCursor = 1; //draw vertical cursor
        gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
      }
    }

  }else{
    gtk_label_set_text(bottom_info_text,"Drag spectrum to pan, mouse wheel to zoom.");
    if(guiglobals.drawSpCursor == 1){
      guiglobals.drawSpCursor = 0; //hide vertical cursor
      gtk_widget_queue_draw(GTK_WIDGET(spectrum_drawing_area));
    }
  }

  return;
}

//get the bin position in the histogram plot
float getXPos(const int bin, const float width, const float xorigin){
  int binc=bin;
  if(bin < 0)
    binc=0;
  else if(bin > (drawing.upperLimit - drawing.lowerLimit))
    binc=drawing.upperLimit - drawing.lowerLimit;
  //printf("bin: %i\n",bin);
  return xorigin + (binc*(width-xorigin)/(drawing.upperLimit-drawing.lowerLimit));
}

//get the screen position of a channel (or fractional channel)
//returns -1 if offscreen
//if halfBinOffset=1, will offset by half a bin (for drawing fits)
float getXPosFromCh(const float chVal, const float width, const unsigned char halfBinOffset, const float xorigin){
  if((chVal < drawing.lowerLimit)||(chVal > drawing.upperLimit)){
    return -1;
  }
  float bin = chVal - drawing.lowerLimit;
  if(halfBinOffset)
    bin += (drawing.contractFactor/2.);
  return xorigin + (bin*(width-xorigin)/(drawing.upperLimit-drawing.lowerLimit));
}

//get the y-coordinate for drawing a specific bin value
float getYPos(const float val, const int multiplotSpNum, const float height, const float yorigin){
  float pos, minVal;
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      minVal = pos = yorigin + (height-yorigin)*(multiplotSpNum/(drawing.numMultiplotSp*1.0));
      if(drawing.logScale){
        if((val > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            pos = yorigin + (height-yorigin)*((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*(log10(val - drawing.scaleLevelMin[multiplotSpNum])/log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])));
          }else{
            pos = yorigin + (height-yorigin)*((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*(log10(val)/log10(drawing.scaleLevelMax[multiplotSpNum])));
          }
        }else{
          pos = minVal;
        }
      }else{
        pos = yorigin + (height-yorigin)*((multiplotSpNum/(drawing.numMultiplotSp*1.0)) + (1.0/(drawing.numMultiplotSp*1.0))*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])));
      }
      //clip value to bottom edge of plot
      if((pos < minVal)||(pos!=pos))
        pos = minVal;
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      //single plot
      if(drawing.logScale){
        if((val > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            pos = yorigin + (height-yorigin)*(log10(val - drawing.scaleLevelMin[multiplotSpNum])/log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]));
          }else{
            pos = yorigin + (height-yorigin)*(log10(val)/log10(drawing.scaleLevelMax[multiplotSpNum]));
          } 
        }else{
          pos = yorigin;
        }
      }else{
        pos = yorigin + (height-yorigin)*((val - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]));
      }
      //clip value to bottom edge of plot
      if((pos < yorigin)||(pos!=pos))
        pos = yorigin;
      break;
  }
  //printf("pos: %f\n",pos);
  return pos;
}

//axis tick drawing
float getAxisXPos(const int axisVal, const float width, const float xorigin){
  int cal_lowerLimit = drawing.lowerLimit;
  int cal_upperLimit = drawing.upperLimit;
  if(calpar.calMode==1){
    //calibrate
    cal_lowerLimit = getCalVal(drawing.lowerLimit);
    cal_upperLimit = getCalVal(drawing.upperLimit);
  }
  if(((cal_upperLimit>cal_lowerLimit)&&((axisVal < cal_lowerLimit)||(axisVal >= cal_upperLimit))) || ((cal_lowerLimit>cal_upperLimit)&&((axisVal > cal_lowerLimit)||(axisVal <= cal_upperLimit))))
    return (float)SMALL_NUMBER; //value is off the visible axis
  
  return xorigin + (width-xorigin)*(axisVal - cal_lowerLimit)/(cal_upperLimit - cal_lowerLimit);
}
void drawXAxisTick(const int axisVal, cairo_t *cr, const float width, const float height, const double baseFontSize, const float xorigin, const float yorigin){
  float axisPos = getAxisXPos(axisVal,width,xorigin);
  //printf("axis Val: %i, axisPos: %f\n",axisVal,axisPos);
  if(axisPos != (float)SMALL_NUMBER){
    //axis position is valid
    cairo_move_to (cr, (int)axisPos, -yorigin);
    cairo_line_to (cr, (int)axisPos, -yorigin*0.875);
    char tickLabel[20];
    sprintf(tickLabel,"%i",axisVal); //set string for label
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, (int)axisPos - extents.width/2., -yorigin*0.5);
    cairo_show_text(cr, tickLabel);
  }
}
float getAxisYPos(const float axisVal, const int multiplotSpNum, const float height, const float yorigin){
  float posVal;
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      if(drawing.logScale){
        if((axisVal > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            posVal = (1.0/drawing.numMultiplotSp)*(yorigin-height)*log10(axisVal - drawing.scaleLevelMin[multiplotSpNum])/log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) + (yorigin-height)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - yorigin;
          }else{
            posVal = (1.0/drawing.numMultiplotSp)*(yorigin-height)*log10(axisVal)/log10(drawing.scaleLevelMax[multiplotSpNum]) + (yorigin-height)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - yorigin;
          }
        }else{
          posVal = (yorigin-height)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - yorigin;
        }
      }else{
        posVal = (1.0/drawing.numMultiplotSp)*(yorigin-height)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) + (yorigin-height)*(multiplotSpNum/(drawing.numMultiplotSp*1.0)) - yorigin;
      }
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      if(drawing.logScale){
        if((axisVal > 0)&&(drawing.scaleLevelMax[multiplotSpNum] > 0)){
          if(drawing.scaleLevelMin[multiplotSpNum] > 0){
            posVal = (yorigin-height)*log10(axisVal - drawing.scaleLevelMin[multiplotSpNum])/log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) - yorigin;
          }else{
            posVal = (yorigin-height)*log10(axisVal)/log10(drawing.scaleLevelMax[multiplotSpNum]) - yorigin;
          }    
        }else{
          posVal = -yorigin;
        }
      }else{
        posVal = (yorigin-height)*(axisVal - drawing.scaleLevelMin[multiplotSpNum])/(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum]) - yorigin;
      }
      break;
  }

  //printf("height: %f, multiplotsp: %i, axisval: %f, posval: %f\n",height,multiplotSpNum,axisVal,posVal);
  return posVal;
}
void drawYAxisTick(const float axisVal, const int multiplotSpNum, cairo_t *cr, const float width, const float height, const double baseFontSize, const float xorigin, const float yorigin){
  if((axisVal < drawing.scaleLevelMin[multiplotSpNum])||(axisVal >= drawing.scaleLevelMax[multiplotSpNum])){
    //printf("axisval:%f,scalemin:%f,scalemax:%f\n",axisVal,drawing.scaleLevelMin[multiplotSpNum],drawing.scaleLevelMax[multiplotSpNum]);
    return; //invalid axis value,
  }
  if(drawing.logScale == 0){
    if((axisVal!=0.0)&&(fabs(axisVal) < (drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/20.0)){
      //tick is too close to zero, don't draw
      return;
    }
    if((drawing.multiplotMode == 4)&&(fabs(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < ((drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/8.0)) ){
      //tick is too close to the top of the spectrum in a stacked view, don't draw
      return;
    }
  }else{
    if(axisVal <= 0.){
      return; //invalid axis value in log scale
    }
    if(drawing.scaleLevelMin[multiplotSpNum] > 0.){
      if((log10(axisVal) < log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/20.0)){
        //tick is too close to zero, don't draw
        return;
      }
      if((drawing.multiplotMode == 4)&&(log10(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < (log10(drawing.scaleLevelMax[multiplotSpNum] - drawing.scaleLevelMin[multiplotSpNum])/8.0)) ){
        //tick is too close to the top of the spectrum in a stacked view, don't draw
        return;
      }
    }else{
      if((log10(axisVal) < log10(drawing.scaleLevelMax[multiplotSpNum])/20.0)){
        //tick is too close to zero, don't draw
        return;
      }
      if((drawing.multiplotMode == 4)&&(log10(drawing.scaleLevelMax[multiplotSpNum] - axisVal) < (log10(drawing.scaleLevelMax[multiplotSpNum])/8.0)) ){
        //tick is too close to the top of the spectrum in a stacked view, don't draw
        return;
      }
    }
    
  }
  
  float axisPos = getAxisYPos(axisVal,multiplotSpNum,height,yorigin);
  if((axisPos <= 0) && (axisPos > (height)*-0.98)) {
    //axis position is valid (ie. on the plot, and not too close to the top of the plot so that it won't be cut off)
    cairo_move_to (cr, xorigin*1.06, axisPos);
    cairo_line_to (cr, xorigin*0.94, axisPos);
    char tickLabel[20];
    getFormattedYAxisVal(axisVal, drawing.scaleLevelMin[multiplotSpNum], drawing.scaleLevelMax[multiplotSpNum], tickLabel, 20);
    
    cairo_text_extents_t extents; //get dimensions needed to center text labels
    cairo_text_extents(cr, tickLabel, &extents);
    cairo_set_font_size(cr, baseFontSize);
    cairo_move_to(cr, xorigin*0.875 - extents.width, axisPos + extents.height/2.);
    cairo_show_text(cr, tickLabel);
  }
}

void drawPlotLabel(cairo_t *cr, const float width, const float height, const double baseFontSize, const float yorigin){
  char plotLabel[256];
  int i;
  cairo_text_extents_t extents; //get dimensions needed to justify text labels
  float labelYOffset;
  cairo_set_font_size(cr, baseFontSize);
  switch(drawing.multiplotMode){
    case 4:
      //stacked spectra
      labelYOffset = height/(3.*drawing.numMultiplotSp);
      if(labelYOffset > yorigin){
        labelYOffset = yorigin;
      }
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width, (height-yorigin)*((drawing.numMultiplotSp-i-1)/(drawing.numMultiplotSp*1.0)) + labelYOffset);
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 3:
    case 2:
      //overlaid spectra
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width, yorigin*(1.0 + 0.45*i));
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 1:
      //summed spectra
      setTextColor(cr);
      strcpy(plotLabel, "Sum of:");
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (width)*0.95 - extents.width, yorigin);
      cairo_show_text(cr, plotLabel);
      for(i=0;i<drawing.numMultiplotSp;i++){
        if(drawing.scaleFactor[drawing.multiPlots[i]] == 1.0){
          strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[i]]);
        }else{
          snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[i]],drawing.scaleFactor[drawing.multiPlots[i]]);
        }
        cairo_text_extents(cr, plotLabel, &extents);
        cairo_move_to(cr, (width)*0.95 - extents.width,  yorigin*(1.0 + 0.45*(i+1)));
        cairo_show_text(cr, plotLabel);
      }
      break;
    case 0:
      //single plot mode
      setTextColor(cr);
      if(drawing.scaleFactor[drawing.multiPlots[0]] == 1.0){
        strcpy(plotLabel, rawdata.histComment[drawing.multiPlots[0]]);
      }else{
        snprintf(plotLabel,256,"%s (scaled by %.2f)",rawdata.histComment[drawing.multiPlots[0]],drawing.scaleFactor[drawing.multiPlots[0]]);
      }
      cairo_text_extents(cr, plotLabel, &extents);
      cairo_move_to(cr, (width)*0.95 - extents.width, yorigin);
      cairo_show_text(cr, plotLabel);
      break;
    default:
      break;
  }
}

float getDistBetweenYAxisTicks(const float axisRange, const int numTicks){

  if(axisRange == 0){
    return 1;
  }

  float trialDist = 0;
  int sigf = 0;
  if(axisRange < 1){
    //get number of sig figs
    float axisRangec = axisRange;
    while(axisRangec < 1){
      axisRangec *= 10.;
      sigf--;
    }
  }else{
    //get number of sig figs
    float axisRangec = 1;
    while(axisRangec < axisRange){
      axisRangec *= 10.;
      sigf++;
    }
  }

  trialDist =  pow(10,sigf-1);
  int trialNumTicks = (int)floor(axisRange/trialDist);
  //printf("1 - axisRange: %f, sigf: %i, trialDist: %f, trialNumTicks: %i, numTicks: %i\n",axisRange,sigf,trialDist,trialNumTicks,numTicks);

  if(trialNumTicks > numTicks){
    if((int)floor(axisRange/(trialDist*5.0)) < (numTicks/1.5))
      trialDist *= 2.0;
    else
      trialDist *= 5.0;
  }else{
    if((int)floor(axisRange/(trialDist/5.0)) > numTicks*1.5)
      trialDist /= 2.0;
    else
      trialDist /= 5.0;
  }
  //printf("2 - axisRange: %f, sigf: %i, trialDist: %f, trialNumTicks: %i, numTicks: %i\n",axisRange,sigf,trialDist,trialNumTicks,numTicks);

  return ceil(trialDist);

}

float getDistBetweenXAxisTicks(const float axisRange){
  //use custom values
  if(axisRange > 20000){
    return 5000;
  }else if(axisRange > 10000){
    return 2000;
  }else if(axisRange > 5000){
    return 1000;
  }else if(axisRange > 3000){
    return 500;
  }else if(axisRange > 2000){
    return 300;
  }else if(axisRange > 1000){
    return 200;
  }else if(axisRange > 500){
    return 100;
  }else if(axisRange > 200){
    return 50;
  }else if(axisRange > 100){
    return 20;
  }else if(axisRange > 50){
    return 10;
  }else if(axisRange > 20){
    return 5;
  }else if(axisRange >= 0){
    return 2;
  }else if(axisRange > -20){
    return -2;
  }else if(axisRange > -50){
    return -5;
  }else if(axisRange > -100){
    return -10;
  }else if(axisRange > -200){
    return -20;
  }else if(axisRange > -500){
    return -50;
  }else if(axisRange > -1000){
    return -100;
  }else if(axisRange > -2000){
    return -200;
  }else if(axisRange > -3000){
    return -300;
  }else if(axisRange > -5000){
    return -500;
  }else if(axisRange > -10000){
    return -1000;
  }else if(axisRange > -20000){
    return -2000;
  }else{
    return -5000;
  }
}

//get the x range of the plot in terms of x axis units, 
//taking into account whether or not a calibration is in use
int getPlotRangeXUnits(){
  int cal_lowerLimit = drawing.lowerLimit;
  int cal_upperLimit = drawing.upperLimit;
  if(calpar.calMode==1){
    //calibrate
    cal_lowerLimit = getCalVal(drawing.lowerLimit);
    cal_upperLimit = getCalVal(drawing.upperLimit);
  }
  return cal_upperLimit - cal_lowerLimit;
}


//draw a spectrum
//drawLabels: 0=don't draw, 1=draw
//showFit: 0=don't show, 1=show without highlighted peaks, 2=show with highlighted peaks
//drawComments: 0=don't draw, 1=draw
//drawFast: 0=don't interpolate, 1=interpolate (faster drawing, less accurate)
void drawSpectrum(cairo_t *cr, const float width, const float height, const float scaleFactor, const unsigned char drawLabels, const unsigned char showFit, const unsigned char drawComments, const unsigned char drawFast){

  if(!rawdata.openedSp){
    return;
  }

  if (drawing.multiPlots[0] >= NSPECT)
  {
    printf("Spectrum number too high (%i)!\n", drawing.multiPlots[0]);
    return;
  }

  int i,j,k;

  //set the origin of the coordinate system in pixels
  float xorigin = 80.0*scaleFactor;
  float yorigin = 40.0*scaleFactor;
  
  // Draw the background colour
  //cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  //cairo_paint(cr);

  //printf("width: %f, height: %f\n",width,height);
  cairo_set_line_width(cr, 2.0*scaleFactor);

  double plotFontSize = 13.5*scaleFactor;

  //draw label(s) for the plot
  if(drawLabels){
    setTextColor(cr);
    drawPlotLabel(cr, width, height, plotFontSize, yorigin); //draw plot label(s)
    cairo_stroke(cr);
  }

  // transform the coordinate system
  cairo_translate(cr, 0.0, height); //so that the origin is at the lower left
  cairo_scale(cr, 1.0, -1.0); //so that positive y values go up

  setPlotLimits(); //setup the x range to plot over

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
          if(drawing.logScale){
            drawing.scaleToLevelMax[i] = maxVal[i]*2.0;
          }else{
            drawing.scaleToLevelMax[i] = maxVal[i]*1.2;
          }
          drawing.scaleToLevelMin[i] = minVal[i];
        }
        break;
      case 2:
      case 1:
      case 0:
        if(drawing.logScale){
          drawing.scaleToLevelMax[0] = maxVal[0]*2.0;
        }else{
          drawing.scaleToLevelMax[0] = maxVal[0]*1.2;
        }
        drawing.scaleToLevelMin[0] = minVal[0];
        break;
      default:
        break;
    }
    if(guiglobals.useZoomAnimations){
      for(i=0;i<drawing.numMultiplotSp;i++){
        if(drawing.scaleLevelMax[i]==drawing.scaleLevelMin[i]){
          drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
          drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
        }else if((drawing.scaleLevelMax[i] != drawing.scaleToLevelMax[i])||(drawing.scaleLevelMin[i] != drawing.scaleToLevelMin[i])){
          if(drawing.zoomingY == 0){
            //printf("Starting y zoom.\n");
            drawing.zoomingY = 1;
            gtk_widget_add_tick_callback(GTK_WIDGET(spectrum_drawing_area), zoom_y_callback, NULL, NULL);
            break;
          }
        }
      }
    }else{
      for(i=0;i<drawing.numMultiplotSp;i++){
        drawing.scaleLevelMax[i] = drawing.scaleToLevelMax[i];
        drawing.scaleLevelMin[i] = drawing.scaleToLevelMin[i];
      }
    }
  }
  //printf("maxVal = %f, minVal = %f\n",maxVal[0],minVal[0]);
  
  //interpolate (ie. limit the number of bins drawn) in the next step, 
  //to help drawing performance
  int maxDrawBins;
  switch(drawFast){
    case 1:
      maxDrawBins = width*1.5;
      break;
    case 0:
    default:
      maxDrawBins = S32K;
      break;
  }
   
  //printf("maximum bins to draw: %i\n",maxDrawBins);
  int binSkipFactor = (drawing.upperLimit-drawing.lowerLimit)/(maxDrawBins);
  if(binSkipFactor <= drawing.contractFactor)
    binSkipFactor = drawing.contractFactor;
  //printf("binskipfactor: %i, contractFactor: %i\n",binSkipFactor,drawing.contractFactor);
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
        for(j=startBin;j<(drawing.upperLimit-drawing.lowerLimit);j+=binSkipFactor){

          float currentVal, nextVal;

          //draw high values even if they were going to be interpolated over
          if(binSkipFactor > drawing.contractFactor){
            for(k=0;k<binSkipFactor;k++){
              if(getDispSpBinVal(i, j+k) > drawing.scaleLevelMax[i]*0.8){
                currentVal = getDispSpBinVal(i, j);
                nextVal = getDispSpBinVal(i, j+k);
                cairo_move_to (cr, getXPos(j,width,xorigin), getYPos(currentVal,i,height,yorigin));
                cairo_line_to (cr, getXPos(j+k,width,xorigin), getYPos(currentVal,i,height,yorigin));
                cairo_line_to (cr, getXPos(j+k,width,xorigin), getYPos(nextVal,i,height,yorigin));
                break;
              }
            }
          }

          currentVal = getDispSpBinVal(i, j);
          nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_move_to (cr, getXPos(j,width,xorigin), getYPos(currentVal,i,height,yorigin));
          cairo_line_to (cr, getXPos(j+binSkipFactor,width,xorigin), getYPos(currentVal,i,height,yorigin));
          cairo_line_to (cr, getXPos(j+binSkipFactor,width,xorigin), getYPos(nextVal,i,height,yorigin));
        }
        //choose color
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case 2:
      //overlay (common scaling)
      for(i=0;i<drawing.numMultiplotSp;i++){
        for(j=startBin;j<(drawing.upperLimit-drawing.lowerLimit);j+=binSkipFactor){

          float currentVal, nextVal;

          //draw high values even if they were going to be interpolated over
          if(binSkipFactor > drawing.contractFactor){
            for(k=0;k<binSkipFactor;k++){
              if(getDispSpBinVal(i, j+k) > drawing.scaleLevelMax[0]*0.8){
                currentVal = getDispSpBinVal(i, j);
                nextVal = getDispSpBinVal(i, j+k);
                cairo_move_to (cr, getXPos(j,width,xorigin), getYPos(currentVal,0,height,yorigin));
                cairo_line_to (cr, getXPos(j+k,width,xorigin), getYPos(currentVal,0,height,yorigin));
                cairo_line_to (cr, getXPos(j+k,width,xorigin), getYPos(nextVal,0,height,yorigin));
                break;
              }
            }
          }

          currentVal = getDispSpBinVal(i, j);
          nextVal = getDispSpBinVal(i, j+binSkipFactor);
          cairo_move_to (cr, getXPos(j,width,xorigin), getYPos(currentVal,0,height,yorigin));
          cairo_line_to (cr, getXPos(j+binSkipFactor,width,xorigin), getYPos(currentVal,0,height,yorigin));
          cairo_line_to (cr, getXPos(j+binSkipFactor,width,xorigin), getYPos(nextVal,0,height,yorigin));
        }
        //choose color
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        cairo_stroke(cr);
      }
      break;
    case 1:
      //summed
    case 0:
      for(i=startBin;i<(drawing.upperLimit-drawing.lowerLimit);i+=binSkipFactor){

        float currentVal, nextVal;

        //draw high values even if they were going to be interpolated over
        if(binSkipFactor > drawing.contractFactor){
          for(k=0;k<binSkipFactor;k++){
            if(getDispSpBinVal(0, i+k) > drawing.scaleLevelMax[0]*0.8){
              currentVal = getDispSpBinVal(0, i);
              nextVal = getDispSpBinVal(0, i+k);
              cairo_move_to (cr, getXPos(i,width,xorigin), getYPos(currentVal,0,height,yorigin));
              cairo_line_to (cr, getXPos(i+k,width,xorigin), getYPos(currentVal,0,height,yorigin));
              cairo_line_to (cr, getXPos(i+k,width,xorigin), getYPos(nextVal,0,height,yorigin));
              break;
            }
          }
        }

        currentVal = getDispSpBinVal(0, i);
        nextVal = getDispSpBinVal(0, i+binSkipFactor);
        //printf("Here! x=%f,y=%f,yorig=%f xclip=%f %f\n",getXPos(i,width), rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],rawdata.hist[drawing.multiPlots[0]][drawing.lowerLimit+i],0,width);
        cairo_move_to (cr, getXPos(i,width,xorigin), getYPos(currentVal,0,height,yorigin));
        cairo_line_to (cr, getXPos(i+binSkipFactor,width,xorigin), getYPos(currentVal,0,height,yorigin));
        cairo_line_to (cr, getXPos(i+binSkipFactor,width,xorigin), getYPos(nextVal,0,height,yorigin));
      }
      cairo_set_source_rgb (cr, drawing.spColors[0], drawing.spColors[1], drawing.spColors[2]);
      cairo_stroke(cr);
      break;
    default:
      break;
  }

  //draw fit
  if((guiglobals.fittingSp == 6)&&(showFit>0)){
    if((drawing.lowerLimit < fitpar.fitEndCh)&&(drawing.upperLimit > fitpar.fitStartCh)){
      cairo_set_line_width(cr, 3.0*scaleFactor);
      cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
      float fitDrawX, nextFitDrawX, xpos, nextXpos;
      //draw each peak
      for(i=0;i<fitpar.numFitPeaks;i++){
        for(fitDrawX=fitpar.fitStartCh; fitDrawX<=fitpar.fitEndCh; fitDrawX+= 0.5){
          nextFitDrawX = fitDrawX + 0.5;
          xpos = getXPosFromCh(fitDrawX,width,1,xorigin);
          nextXpos = getXPosFromCh(nextFitDrawX,width,1,xorigin);
          if((xpos > 0)&&(nextXpos > 0)){
            cairo_move_to (cr, xpos, getYPos(evalFitOnePeak(fitDrawX,i,fitpar.fitType),0,height,yorigin));
            cairo_line_to (cr, nextXpos, getYPos(evalFitOnePeak(nextFitDrawX,i,fitpar.fitType),0,height,yorigin));
          }
        }
      }
      //draw background
      for(fitDrawX=fitpar.fitStartCh; fitDrawX<=fitpar.fitEndCh; fitDrawX+= 0.5){
        nextFitDrawX = fitDrawX + 0.5;
        xpos = getXPosFromCh(fitDrawX,width,1,xorigin);
        nextXpos = getXPosFromCh(nextFitDrawX,width,1,xorigin);
        if((xpos > 0)&&(nextXpos > 0)){
          cairo_move_to (cr, xpos, getYPos(evalFitBG(fitDrawX),0,height,yorigin));
          cairo_line_to (cr, nextXpos, getYPos(evalFitBG(nextFitDrawX),0,height,yorigin));
        }
      }
      cairo_stroke(cr);
      //draw sum of peaks
      if(fitpar.numFitPeaks > 1){
        cairo_set_line_width(cr, 2.0*scaleFactor);
        for(fitDrawX=fitpar.fitStartCh; fitDrawX<=fitpar.fitEndCh; fitDrawX+= 0.5){
          nextFitDrawX = fitDrawX + 0.5;
          xpos = getXPosFromCh(fitDrawX,width,1,xorigin);
          nextXpos = getXPosFromCh(nextFitDrawX,width,1,xorigin);
          if((xpos > 0)&&(nextXpos > 0)){
            cairo_move_to (cr, xpos, getYPos(evalFit(fitDrawX,fitpar.fitType),0,height,yorigin));
            cairo_line_to (cr, nextXpos, getYPos(evalFit(nextFitDrawX,fitpar.fitType),0,height,yorigin));
          }
        }
        cairo_stroke(cr);
      }
      //draw highlighed peak
      if((drawing.highlightedPeak >= 0)&&(drawing.highlightedPeak <= fitpar.numFitPeaks)&&(showFit>1)){
        cairo_set_line_width(cr, 6.0*scaleFactor);
        for(fitDrawX=floorf(fitpar.fitParVal[7+(3*drawing.highlightedPeak)] - 3.*fitpar.fitParVal[8+(3*drawing.highlightedPeak)]);  fitDrawX<=floorf(fitpar.fitParVal[7+(3*drawing.highlightedPeak)] + 3.*fitpar.fitParVal[8+(3*drawing.highlightedPeak)]); fitDrawX+= 0.5){
          nextFitDrawX = fitDrawX + 0.5;
          xpos = getXPosFromCh(fitDrawX,width,1,xorigin);
          nextXpos = getXPosFromCh(nextFitDrawX,width,1,xorigin);
          if((xpos > 0)&&(nextXpos > 0)){
            cairo_move_to (cr, xpos, getYPos(evalFitOnePeak(fitDrawX,drawing.highlightedPeak,fitpar.fitType),0,height,yorigin));
            cairo_line_to (cr, nextXpos, getYPos(evalFitOnePeak(nextFitDrawX,drawing.highlightedPeak,fitpar.fitType),0,height,yorigin));
          }
        }
        cairo_stroke(cr);
      }
    }
  }

  // draw axis lines
  cairo_set_line_width(cr, 1.0*scaleFactor);
  setTextColor(cr);
  cairo_move_to (cr, xorigin, yorigin);
  cairo_line_to (cr, xorigin, height);
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      for(i=0;i<drawing.numMultiplotSp;i++){
        cairo_move_to (cr, xorigin, yorigin + (height-yorigin)*i/(drawing.numMultiplotSp*1.0));
        cairo_line_to (cr, width, yorigin + (height-yorigin)*i/(drawing.numMultiplotSp*1.0));
      }
      break;
    case 3:
    case 2:
    case 1:
    case 0:
    default:
      //single plot
      cairo_move_to (cr, xorigin, yorigin);
      cairo_line_to (cr, width, yorigin);
      break;
  }
  cairo_stroke(cr);
  
  cairo_scale(cr, 1.0, -1.0); //remove axis inversion, so that text is the right way up

  //draw x axis ticks
  float tickDist = getDistBetweenXAxisTicks(getPlotRangeXUnits());
  for(i=0;abs(i)<S32K;i+=tickDist){
    drawXAxisTick(i, cr, width, height, plotFontSize, xorigin, yorigin);
  }
  for(i=0;abs(i)<S32K;i-=tickDist){
    drawXAxisTick(i, cr, width, height, plotFontSize, xorigin, yorigin);
  }
  cairo_stroke(cr);

  //draw y axis ticks
  int numTickPerSp;
  float yTickDist, yTick;
  switch(drawing.multiplotMode){
    case 4:
      //stacked
      numTickPerSp = (height)/(yorigin*drawing.numMultiplotSp);
      if(numTickPerSp < 2)
        numTickPerSp = 2;
      if(drawing.logScale){
        for(i=0;i<drawing.numMultiplotSp;i++){
          float rangeVal = drawing.scaleLevelMax[i] - drawing.scaleLevelMin[i];
          if(rangeVal > drawing.scaleLevelMax[i])
            rangeVal = drawing.scaleLevelMax[i];
          int numTickUsed = 0;
          if(rangeVal >= 1000.){
            //logarithmic scale ticks in base-10
            float tickVal = pow(10.0,getNSigf(drawing.scaleLevelMax[i],10.0));
            while(numTickUsed < numTickPerSp){
              drawYAxisTick(tickVal, i, cr, width, height, plotFontSize, xorigin, yorigin);
              tickVal /= 10.;
              numTickUsed++;
            }
          }else{
            //logarithmic scale ticks in base-2
            float tickVal = pow(2.0,getNSigf(drawing.scaleLevelMax[i],2.0));
            while(numTickUsed < numTickPerSp){
              drawYAxisTick(tickVal, i, cr, width, height, plotFontSize, xorigin, yorigin);
              tickVal /= 2.;
              numTickUsed++;
            }
          }
        }
      }else{
        for(i=0;i<drawing.numMultiplotSp;i++){
          yTickDist = getDistBetweenYAxisTicks(drawing.scaleLevelMax[i] - drawing.scaleLevelMin[i],numTickPerSp);
          cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
          for(yTick=0.;yTick<drawing.scaleLevelMax[i];yTick+=yTickDist){
            drawYAxisTick(yTick, i, cr, width, height, plotFontSize, xorigin, yorigin);
          }
          for(yTick=0.;yTick>drawing.scaleLevelMin[i];yTick-=yTickDist){
            if(yTick != 0)
              drawYAxisTick(yTick, i, cr, width, height, plotFontSize, xorigin, yorigin);
          }
          //drawYAxisTick(0.0, i, cr, width, height, plotFontSize, xorigin, yorigin); //always draw the zero label on the y axis
          cairo_stroke(cr);
          //draw the zero line if applicable
          if((drawing.scaleLevelMin[i] < 0.0) && (drawing.scaleLevelMax[i] > 0.0)){
            cairo_set_line_width(cr, 1.0*scaleFactor);
            cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
            cairo_move_to (cr, xorigin, getAxisYPos(0.0,i,height,yorigin));
            cairo_line_to (cr, width, getAxisYPos(0.0,i,height,yorigin));
            cairo_stroke(cr);
          }
        }
      }
      
      break;
    case 3:
      //overlay (independent scaling)
      for(i=0;i<drawing.numMultiplotSp;i++){
        float labelOffset = 0.4*(i+1)/(drawing.numMultiplotSp*1.);
        cairo_set_source_rgb (cr, drawing.spColors[3*i], drawing.spColors[3*i + 1], drawing.spColors[3*i + 2]);
        drawYAxisTick(drawing.scaleLevelMax[i]*(0.3 + labelOffset), i, cr, width, height, plotFontSize, xorigin, yorigin); //draw one axis tick near the middle of the axis, per spectrum
        drawYAxisTick(0.0, i, cr, width, height, plotFontSize, xorigin, yorigin); //always draw the zero label on the y axis
      }
      break;
    case 2:
    case 1:
    case 0:
      //modes with a single scale
      setTextColor(cr);
      numTickPerSp = height/(2.0*yorigin) + 1;
      if(drawing.logScale){
        //numTickPerSp *= 2;
        int nsigf10 = 0;
        if(drawing.scaleLevelMin[0] <= 0)
          nsigf10 = getNSigf(drawing.scaleLevelMax[0],10.0);
        else
          nsigf10 = getNSigf(drawing.scaleLevelMax[0],10.0) - getNSigf(drawing.scaleLevelMin[0],10.0);
        int numTickUsed = 0;
        //printf("nsigf10: %i\n",nsigf10);
        if(nsigf10 >= 3){
          //logarithmic scale ticks in base-10
          float tickVal = pow(10.0,getNSigf(drawing.scaleLevelMax[0],10.0));
          while(numTickUsed < numTickPerSp){
            drawYAxisTick(tickVal, 0, cr, width, height, plotFontSize, xorigin, yorigin);
            tickVal /= 10.;
            numTickUsed++;
          }
        }else{
          //logarithmic scale ticks in base-2
          float tickVal = pow(2.0,getNSigf(drawing.scaleLevelMax[0],2.0));
          while(numTickUsed < numTickPerSp){
            drawYAxisTick(tickVal, 0, cr, width, height, plotFontSize, xorigin, yorigin);
            tickVal /= 2.;
            numTickUsed++;
          }
        }
      }else{
        yTickDist = getDistBetweenYAxisTicks(drawing.scaleLevelMax[0] - drawing.scaleLevelMin[0],numTickPerSp);
        for(yTick=0.;yTick<drawing.scaleLevelMax[0];yTick+=yTickDist){
          drawYAxisTick(yTick, 0, cr, width, height, plotFontSize, xorigin, yorigin);
        }
        for(yTick=0.;yTick>drawing.scaleLevelMin[0];yTick-=yTickDist){
          if(yTick != 0)
            drawYAxisTick(yTick, 0, cr, width, height, plotFontSize, xorigin, yorigin);
        }
        //printf("min: %f, max: %f, yTickDist: %f, numTickPerSp: %i\n",drawing.scaleLevelMin[0],drawing.scaleLevelMax[0],yTickDist,numTickPerSp);
        //drawYAxisTick(0.0, 0, cr, width, height, plotFontSize, xorigin, yorigin); //always draw the zero label on the y axis
        cairo_stroke(cr);
        //draw the zero line if applicable
        if((drawing.scaleLevelMin[0] < 0.0) && (drawing.scaleLevelMax[0] > 0.0)){
          cairo_set_line_width(cr, 1.0*scaleFactor);
          cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
          cairo_move_to (cr, xorigin, getAxisYPos(0.0,0,height,yorigin));
          cairo_line_to (cr, width, getAxisYPos(0.0,0,height,yorigin));
          cairo_stroke(cr);
        }
      }
      break;
    default:
      break;
  }

  //draw axis labels
  setTextColor(cr);
  char axisLabel[16],axisYLabel[32];
  cairo_text_extents_t extents; //for getting dimensions needed to center text labels
  //x axis
  if(calpar.calMode == 0){
    //set default strings for labels
    sprintf(axisLabel,"Channel #");
    sprintf(axisYLabel,"Value");
  }else{
    //set labels to calibrated units
    strcpy(axisLabel,calpar.calUnit);
    sprintf(axisYLabel,"%s",calpar.calYUnit);
  }
  cairo_text_extents(cr, axisLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, (width)*0.55 - (extents.width/2), -3.0);
  cairo_show_text(cr, axisLabel);
  //y axis
  cairo_text_extents(cr, axisYLabel, &extents);
  cairo_set_font_size(cr, plotFontSize*1.2);
  cairo_move_to(cr, 16.0*scaleFactor, (-height)*0.525 + (extents.width/2));
  cairo_save(cr); //store the context before the rotation
  cairo_rotate(cr, 1.5*3.14159);
  cairo_translate(cr, (width)*0.015, -1.0*((-height)*0.5)); //so that the origin is at the lower left
  cairo_show_text(cr, axisYLabel);
  cairo_stroke(cr);
  cairo_restore(cr); //recall the unrotated context

  //draw fit cursors and indicators
  if((guiglobals.fittingSp > 0)&&(showFit>0)){

    //draw cursors at fit limits if needed
    if(fitpar.fitStartCh >= 0){
      float cursorPos = getXPosFromCh(fitpar.fitStartCh, width, 0, xorigin);
      if(cursorPos>=0){
        cairo_set_line_width(cr, 2.0*scaleFactor);
        cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, cursorPos, -yorigin);
        cairo_line_to(cr, cursorPos, -height);
        cairo_stroke(cr);
      }
    }
    if(fitpar.fitEndCh >= 0){
      float cursorPos = getXPosFromCh(fitpar.fitEndCh, width, 0, xorigin);
      if(cursorPos>=0){
        cairo_set_line_width(cr, 2.0*scaleFactor);
        cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
        cairo_move_to(cr, cursorPos, -yorigin);
        cairo_line_to(cr, cursorPos, -height);
        cairo_stroke(cr);
      }
    }
    //draw peak position indicators
    if((guiglobals.fittingSp >= 2)&&(guiglobals.fittingSp < 6)){
      //put markers at guessed positions
      cairo_set_source_rgb (cr, 0.0, 0.0, 0.8);
      cairo_set_line_width(cr, 2.0*scaleFactor);
      for(i=0;i<fitpar.numFitPeaks;i++){
        if((fitpar.fitPeakInitGuess[i] > drawing.lowerLimit)&&(fitpar.fitPeakInitGuess[i] < drawing.upperLimit)){
          cairo_arc(cr,getXPosFromCh(fitpar.fitPeakInitGuess[i],width,1,xorigin),(-0.002*(height)*30.0)-getYPos(getDispSpBinVal(0,fitpar.fitPeakInitGuess[i]-drawing.lowerLimit),0,height,yorigin),5.,0.,2*G_PI);
        }
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
      }
    }else if(guiglobals.fittingSp == 6){
      //put markers at fitted positions
      cairo_set_source_rgb (cr, 0.0, 0.0, 0.8);
      cairo_set_line_width(cr, 2.0*scaleFactor);
      for(i=0;i<fitpar.numFitPeaks;i++){
        if((fitpar.fitParVal[7+(3*i)] > drawing.lowerLimit)&&(fitpar.fitParVal[7+(3*i)] < drawing.upperLimit)){
          cairo_arc(cr,getXPosFromCh(fitpar.fitParVal[7+(3*i)],width,1,xorigin),(-0.002*(height)*30.0)-getYPos(evalFit(fitpar.fitParVal[7+(3*i)],fitpar.fitType),0,height,yorigin),5.,0.,2*G_PI);
        }
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
      }
    }
  }

  //draw comment indicators
  if(drawComments){
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    switch (drawing.multiplotMode)
    {
      case 1:
        //sum view
        for(i=0;i<rawdata.numChComments;i++){
          if(rawdata.chanCommentView[i]==1){
            if(rawdata.chanCommentSp[i]==drawing.displayedView){
              if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                  if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                    if(drawing.highlightedComment == i){
                      cairo_set_line_width(cr, 8.0*scaleFactor);
                    }else{
                      cairo_set_line_width(cr, 4.0*scaleFactor);
                    }
                    float chYVal = rawdata.chanCommentVal[i];
                    if(chYVal < drawing.scaleLevelMin[0]){
                      chYVal = drawing.scaleLevelMin[0];
                    }else if(chYVal > drawing.scaleLevelMax[0]){
                      chYVal = drawing.scaleLevelMax[0];
                    }
                    float xc = getXPosFromCh(rawdata.chanCommentCh[i],width,1,xorigin);
                    float yc = -1.0*getYPos(chYVal,0,height,yorigin);
                    float radius = 14.0;
                    cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                    cairo_set_font_size(cr, plotFontSize*1.5);
                    cairo_text_extents(cr, "i", &extents);
                    cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                    cairo_show_text(cr, "i");
                    cairo_stroke(cr);
                    //cairo_fill(cr);
                  }
                }
              }
            }
          }
        }
        break;
      case 0:
        //single non-summed spectrum
        if(drawing.displayedView == -1){
          for(i=0;i<rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i]==0){
              if(rawdata.chanCommentSp[i]==drawing.multiPlots[0]){
                if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                  if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                    if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                      if(drawing.highlightedComment == i){
                        cairo_set_line_width(cr, 8.0*scaleFactor);
                      }else{
                        cairo_set_line_width(cr, 4.0*scaleFactor);
                      }
                      float chYVal = rawdata.chanCommentVal[i];
                      if(chYVal < drawing.scaleLevelMin[0]){
                        chYVal = drawing.scaleLevelMin[0];
                      }else if(chYVal > drawing.scaleLevelMax[0]){
                        chYVal = drawing.scaleLevelMax[0];
                      }
                      float xc = getXPosFromCh(rawdata.chanCommentCh[i],width,1,xorigin);
                      float yc = -1.0*getYPos(chYVal,0,height,yorigin);
                      float radius = 14.0;
                      cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                      cairo_set_font_size(cr, plotFontSize*1.5);
                      cairo_text_extents(cr, "i", &extents);
                      cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                      cairo_show_text(cr, "i");
                      cairo_stroke(cr);
                      //cairo_fill(cr);
                    }
                  }
                }
              }
            }
          }
        }else{
          for(i=0;i<rawdata.numChComments;i++){
            if(rawdata.chanCommentView[i]==1){
              if(rawdata.chanCommentSp[i]==drawing.displayedView){
                if(rawdata.chanCommentCh[i] > drawing.lowerLimit){
                  if(rawdata.chanCommentCh[i] < drawing.upperLimit){
                    if((!drawing.logScale)||(rawdata.chanCommentVal[i] > 0)){
                      if(drawing.highlightedComment == i){
                        cairo_set_line_width(cr, 8.0*scaleFactor);
                      }else{
                        cairo_set_line_width(cr, 4.0*scaleFactor);
                      }
                      float chYVal = rawdata.chanCommentVal[i];
                      if(chYVal < drawing.scaleLevelMin[0]){
                        chYVal = drawing.scaleLevelMin[0];
                      }else if(chYVal > drawing.scaleLevelMax[0]){
                        chYVal = drawing.scaleLevelMax[0];
                      }
                      float xc = getXPosFromCh(rawdata.chanCommentCh[i],width,1,xorigin);
                      float yc = -1.0*getYPos(chYVal,0,height,yorigin);
                      float radius = 14.0;
                      cairo_arc(cr,xc,yc,radius,0.,2*G_PI);
                      cairo_set_font_size(cr, plotFontSize*1.5);
                      cairo_text_extents(cr, "i", &extents);
                      cairo_move_to(cr,xc-(extents.width),yc+(extents.height/2.));
                      cairo_show_text(cr, "i");
                      cairo_stroke(cr);
                      //cairo_fill(cr);
                    }
                  }
                }
              }
            }
          }
        }
        break;
      default:
        //comments not implemented
        break;
    }
  }

  //draw cursor at mouse position
  if(guiglobals.drawSpCursor == 1){
    //printf("Drawing cursor!\n");
     cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1.0);
    setTextColor(cr);
    cairo_move_to(cr, guiglobals.cursorPosX, -yorigin);
    cairo_line_to(cr, guiglobals.cursorPosX, -height);
    cairo_stroke(cr);
  }

  return;
}

//update the spectrum drawing area
void drawSpectrumArea(GtkWidget *widget, cairo_t *cr, gpointer user_data){

  if(!rawdata.openedSp){
    return;
  }

  //printf("Drawing spectrum!\n");

  GdkRectangle dasize;  // GtkDrawingArea size
  GdkWindow *wwindow = gtk_widget_get_window(widget);
  // Determine GtkDrawingArea dimensions
  gdk_window_get_geometry(wwindow, &dasize.x, &dasize.y, &dasize.width, &dasize.height);

  drawSpectrum(cr, (float)dasize.width, (float)dasize.height, 1.0, guiglobals.drawSpLabels, 2, guiglobals.drawSpComments, 1);
}