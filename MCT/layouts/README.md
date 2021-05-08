# MCT Layout Creation

## Overview:

There are Many useful tools to create a layout built into OpenMCT that this document will outline.
There are also specific procedural steps we want to take when making layouts so they are created in a uniform fashion.

Note: all steps and instrucions in this document require a working and running OpenMCT enviorment.

## To create a new layout:

1. Click the blue "Create" button
2. Choose either Display Layout or Flexible Layout
3. Name your layout
4. Choose the My Items folder as the folder to save the layout
5. Click ok and wait for it to load the empty layout.
6. At this point, click the save icon on the top of the layout and choose "Save and finish editing"
7. You are now ready to begin adding other Domain Objects to the layout

#### - Display Layout
Description: A layout with less flexibility in the size of its subobects but is still functional

#### - Flexible Layout

Description: A more complicated layout with more flexibility in what can be added to it and the sizes of its subobjects.

## To create a domain object and store it in your layout:
1. Click the blue "Create" button
2. Choose any of the options
3. Fill out the options such as the name
4. Select to save it inside `MyItems/YourLayoutName` (This will let us later export the layout as one complete package rather than multiple parts)
5. Click ok and wait for it to load the empty domain object.
6. At this point, procede to the directions specific to the domain object you are trying to add.
7. NOTE: if your domain object requires/depends upon other domain objects, put it in `MyItems/YourLayoutName/YourDomainObjectName` not `MyItems/YourLayoutName`

## Creating specific domain objects:

#### - Clock:

Description: This is a clock that can display the current time in many formats or timezones.

1. Click the blue "Create" button
2. Choose "Clock"
3. Fill out the options such as the Title, Timezone, Timeformat, and notes
4. Add to a layout or a domain object.
5. select ok

#### - Condition Set:

Description: This is a object that lets you set certain formatting based on conditions of certain telemetry values

1. Click the blue "Create" button
2. Choose "Condition Set"
3. Fill out the options such as the title and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the condition set creation wizard
6. Set the default condition listed at the bottom. You can have it output a string or  boolean (Feel free to change the Condition Name)
7. Click the "Add Condition" button to add a new condition - you can set the output, or name like the defualt, but now you can also set criteria that must be met for it to return this output.
8. in the file tree navigate up to follower or leader folders and select a telemetry point you want this condition to look at.
9. Click and drage the file from the file tree to the body of the condition creation wizard. Now, that telemetry point is available to use in a "select telemetry" drop down
10. Configure the remaining drop down menus so it uses the logic you desire
11. add any remaining conditions or telemetry fields as done before
12. You can also test what the output would be for different fields when making the condition set by creating a test datum above the conditions.
13. When done click save and finish editing.

#### - Condition Widget:

Description: Like a condition set, but it has a viewable status based on a condition set output

1. Click the blue "Create" button
2. Choose "Condition Widget"
3. Fill out the options such as the title, Label, URL, and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the condition widget
6. You can then use a condition set as the input for a condition widget
7. Click save and finish editing when done.

#### - Folder:

Description: A folder to organize domain objects in

1. Click the blue "Create" button
2. Choose "Folder"
3. Fill out the options such as the Title and notes
4. Add to a layout or a domain object.
5. select ok

#### - Hyperlink:

Description: Sets a link to a website - potentially can also link between pages of OpenMCT to create navigation buttons

1. Click the blue "Create" button
2. Choose "Hyperlink"
3. Fill out the options such as the Title, URL, Text to display, Display format, whether to open in a new tab or this tab, and notes
4. Add to a layout or a domain object.
5. select ok

#### - Overlay Plot:

Description: This plot allows you to view multiple domain objects on the same plot overlayed on the same axis.

1. Click the blue "Create" button
2. Choose "Overlay Plot"
3. Fill out the options such as the Title and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the empty plot
6. drag in telemetry fields from the follower and leader folders in the file tree that you want to see on the graph
7. You can customize features of the plot in the properties tab including color, symbols, etc.
8. When finished select save and finish editing

#### - Stacked Plot:

Description: This plot allows you to view multiple domain objects on the same plot stacked above each other on different axis

1. Click the blue "Create" button
2. Choose "Stacked Plot"
3. Fill out the options such as the Title and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the empty plot
6. drag in telemetry fields from the follower and leader folders in the file tree that you want to see on the graph
7. You can customize features of the plot in the properties tab including color, symbols, etc.
8. When finished select save and finish editing

#### - Summary Widget:

Description: Like a condition widget and set, but it has a viewable status light/indicator as well based on certain telemetry domain object values

1. Click the blue "Create" button
2. Choose "Summary Widget"
3. Fill out the options such as the title, URL, whether you want to open in a new tab or this tab, and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the summary widget creation wizard
6. in the file tree navigate up to follower or leader folders and select a telemetry point you want this condition to look at.
7. Click and drag the file from the file tree to the body of the condition creation wizard. Now, that telemetry point is available to use in a "select telemetry" drop down
8. Set the default rule listed at the bottom. 
9. Click the "Add Rule" button to add a new condition rule
10. Configure the remaining drop down menus so it uses the logic you desire
11. add any remaining conditions or telemetry fields as done before
12. You can also test what the output would be for different fields when making the summary widget by creating test values above the conditions.
13. When done click save and finish editing.

#### - Tabs View:

 Description: This lets you have multiple tabs to view multiple pages Ideally this would be used either as the first object in a layout, or above a layout where you put the layouts on each tab

1. Click the blue "Create" button
2. Choose "Tabs View"
3. Fill out the options such as the Title, eager load tabs, and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the empty tabs view
6. drag in telemetry fields from the follower and leader folders in the file tree that you want to see on a tab by hovering over the top blue plus symbol and releasing
8. When finished select save and finish editing

#### - Telemetry Table:

Description: Shows a table of the telemetry values of a telemetry domain object

1. Click the blue "Create" button
2. Choose "Telemetry Table"
3. Fill out the options such as the Title and notes
4. Add to a layout or a domain object.
5. select ok and wait for it to load the empty table object
6. drag in a telemetry field from the follower and leader folders in the file tree that you want to see the telemetry table of
8. When finished select save and finish editing

#### - Timer:

Description: Lets u set a timer for a certain event

1. Click the blue "Create" button
2. Choose "Timer"
3. Fill out the options such as the Title, Target time, Display format, and notes
4. Add to a layout or a domain object.
5. select ok

#### - Web Page:

Description: Loads the contents of a webpage into OpenMCT

1. Click the blue "Create" button
2. Choose "Web Page"
3. Fill out the options such as the Title, URL, and notes
4. Add to a layout or a domain object.
5. select ok

## To Export a Layout:

1. right click on Layout in file tree
2. Choose "Export as JSON"

- Note: Select the top most part of your layout, so it pulls the full package. Don't just export a piece inside of your layout.

3. Save the layout in the `FlightSoftware/MCT/layouts `directory 
4. Do not change the name of the json as the name must match the name of the domain object inside the json file

## To Import a Layout:

1. Right Click on My Items
2. Choose "Import as JSON"
3. Select "Select File"
4. Navigate in your OS's file explorer to locate a Layout/Domain Object JSON file that either you previously exported or that someone else added to `FlightSoftware/MCT/layouts` 
5. select ok and wait for the domain object to load into OpenMCT

