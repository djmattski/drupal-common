<?php
/**
 * @file
 * common.inc
 *
 * Contains common functionality for the entire base theme.
 */


/**
* Handle form messages
* @param $formName (string) - name of the form
* @return $msgs (string) - MARKUP for displaying errors
*/
function formMessages($formName){
    $form1 = drupal_get_form($formName);
    $messages = drupal_get_messages();
    $msgs = '';
    if(count($messages)>0){
        if(array_key_exists('error',$messages)){
            $msgs .= '<div id="messages">';
            $msgs .= '<div id="messageHolder">';
            //$msgs .= '<div id="messageCloseBar" class="error"><a href="javascript:void(0);" id="closeMessageBar">Close</a></div>';
            $msgs .= '<div class="messages error">';
            $msgs .= 'Please fix up the errors below and re-submit';
            $msgs .= '<ul>';
            $tempArr = $messages['error'];
            foreach($tempArr as $value){
                $msgs .= '<li>'.$value.'</li>';
            }
            $msgs .= '</ul>';
            $msgs .= '<a class="formJumper" href="#formJump">Jump to the form</a>';
            $msgs .= '</div>';
            $msgs .= '</div>';
            $msgs .= '</div>';
        }
        //$variables['formMsgs'] = $msgs;
    }

    return $msgs;
}

/**
*Function to check for ._ files and folders
*/
function dotCheck($pattern,$entry){
    $i = 0;
    $i = preg_match($pattern,$entry);
    return $i;
}

/**
* Return a clean snippet of text
* @param $text (String) : The string of html to truncate
* @param $length (int) : The length to truncate at
* @return $html (string): Markup
*/
function returnSnippet($text = '', $length = 180){

    global $base_url;
    $html = '';
    
    if($text != ''){
        if (strlen($text) > $length) {
            do{
                //Reset vars
                $pos = 0;
                $nextPos = 0;
                //
                $pos = strpos($text,'<');
                $nextPos = strpos($text,'>');
                $strlength = ($nextPos > 0)? ($nextPos+1) - $pos : 0;
                $text = substr_replace($text,' ',$pos,$strlength);
            }while(strpos($text,'<') !== false);

            $subStr = substr($text,0,$length);
            $spacePos = strrpos($subStr,' ');
            $subStr2 = substr($subStr,0,$spacePos);

            
            $html = $subStr2.'...';
        }else{
            $html = $text;
        }
    }
    

    return $html;
}

/**
* Return information about an image
* @param $fid - the file id of the image
* @param $altField - the field name containing the alt name if it exists
* @param $imageStyle (string) - Name of a predefined style to use
* @param $dimensions (array) - array of dimensions for the style width, height
* @return An object with image properties
*/
function returnImg($fid, $altField = '', $style = 'image', $dimensions = NULL){
    global $base_url;
    $imgObj = new StdClass;
    $imgObj->imgHtml = '';
    $imgObj->uri = '';
    $imgObj->alt = '';
    $imgObj->fileName = '';
    $imgObj->filePath = '';
    $imgObj->stylePath = '';
    $imgObj->dimensions = array('100%','100%');

    $sql = "SELECT fm.uri, altField.".$altField."_alt AS theAlt ".
            "FROM {file_managed} AS fm ".
            "LEFT JOIN {field_data_".$altField."} AS altField ON(altField.".$altField."_fid = fm.fid) ".
            //"WHERE altField.".$altField."_fid = :fid";
            "WHERE fm.fid = :fid";
    $result = db_query($sql, array(':fid' => $fid));
    $number_of_rows = $result->rowCount();
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $variables = array(
              'path' => $record->uri,
              'alt' => $record->theAlt,
              'title' => '',
              'attributes' => array('class'=>'fieldImage'),
            );
            $slashPos = strpos($record->uri,'//');
            $tmpFileName = substr($record->uri,$slashPos+2);
            $tmpFile = drupal_realpath($record->uri);
            if ($style != 'image' && $dimensions != NULL) {
                $imgObj->imgHtml = theme_image_style(array('style_name' => $style, 'path' => $record->uri, 'alt' => $record->theAlt, 'title' => '', 'width' => $dimensions[0], 'height' => $dimensions[1], 'attributes' => array()));
                $imgObj->stylePath = 'styles/'.$style.'/public/'.$tmpFileName;           
            }else{
                $imgObj->imgHtml = theme('image', $variables);
            }
            $imgObj->uri = $record->uri;
            $imgObj->alt = $record->theAlt;
            $imgObj->fileName = $tmpFileName;
            $imgObj->filePath = $tmpFile;
            if (is_file($imgObj->filePath)) {
                $imgObj->dimensions = getimagesize($tmpFile);
            }else{
                $imgObj->dimensions = array('100%','100%');
            }
        }
    }
    return $imgObj;
}

/**
* Return a file details
* @param $uri: the uri of the file
*
* @return: object
*/
function fileDetails($uri){
    $fileOb = new StdClass();
    $slashPos = strpos($uri,'//');
    $tmpFileName = substr($uri,$slashPos+2);
    $extensionPos = strrpos($tmpFileName,'.');
    $extension = strtolower(substr($tmpFileName,$extensionPos+1));
    $tmpFile = drupal_realpath($uri);
    if (file_exists($tmpFile)) {
        $fileSize = ' ';
        $fileSize .= number_format(filesize($tmpFile)/1000,0);
        $fileSize .= 'KB';
    }else{
        $fileSize = '';
    }
    $fileDescript = '';
    switch($extension){
        case 'txt':
            $fileDescript = '(Plain text'.$fileSize.')';
        break;
        case 'doc':
        case 'docx':
            $fileDescript = '(Word document'.$fileSize.')';
        break;
        case 'ppt':
            $fileDescript = '(Power Point presentation'.$fileSize.')';
        break;
        case 'xls':
        case 'xlsx':
            $fileDescript = '(Excel spreadsheet'.$fileSize.')';
        break;
        case 'pdf':
            $fileDescript = '(PDF'.$fileSize.')';
        break;
        default:
            $fileDescript = '';
        break;
    }

    $fileOb->description = $fileDescript;
    $fileOb->fileNameLocation = str_replace(' ', '%20', $tmpFileName);

    return $fileOb;
}

/**
* Return a link to a node if it has been tagged with the same tid
* @param $tid: taxonomy term id
* @param $vid: taxonomy vocab id
* @param $name: taxonomy term name
* @param $table: relevant table in db
* @return $html (string): Markup
*/
function taxonomyLink($tid,$vid,$name,$table){
    global $base_url;
    $html = '';
    $sql = "SELECT n.nid ".
            "FROM {node} AS n ".
            "LEFT JOIN {field_data_field_".$table."} AS ftsc ON(ftsc.entity_id = n.nid) ".
            "WHERE n.status = 1 AND n.type = :type AND ftsc.field_".$table."_tid = :tid";
    $result = db_query($sql, array(':type' => 'expertise', ':tid' => $tid));
    $number_of_rows = $result->rowCount();
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $alias = drupal_lookup_path('alias', 'node/'.$record->nid);
            $html .= '<a class="tagLink" href="'.$base_url.'/'.$alias.'">'.$name.'</a>';
        }
    }else{
        $html .= $name;
    }
    /*
    $html .= '<div>'.$tid.'</div>';
    $html .= '<div>'.$table.'</div>';
    $html .= '<div>'.$sql.'</div>';
    */
    return $html;
}


/**
 * Encode an email address to display
 * @param $email (String) : The email to encode
 * @return String
 */
function encode_email_address($email){
    $output = '';
    for ($i = 0; $i < strlen($email); $i++) {
        $output .= '&#'.ord($email[$i]).';';
    }
    return $output;
}

/**
* Check for a session
* @param $arrayKey (string): Array key of the session var to check
* @return Value of Session
*/
function checkSession($arrayKey){
    $sessionArr = [];
    if (drupal_session_started()) {
      if (array_key_exists($arrayKey, $_SESSION)) {
        $sessionArr = $_SESSION[$arrayKey];
      }
    }
    return $sessionArr;
}

/**
* Clear the $_SESSION var
* @param $sessionKey (string)
* @return Array
*/
function clearSession($sessionKey){
    $sessionArr = checkSession($sessionKey);
    if (!empty($sessionArr)) {
        $_SESSION[$sessionKey] = [];
    }
}

/**
* Remove 2 last char from end of a comma separated list ', '
* @param $list (String) : The comma separated list
* @return String
*/
function removeComma($list){
    $list = substr($list, 0, -2);
    return $list;
}

/**
* Return a taxonomy list that can be rendered in a form
* @param $vid (int) - The vocab ID for the terms to return
* @param $firstTerm (String) - The first item that should show in the list
* @return Object
*/
function taxonomyList($vid,$firstTerm){
    //List terms
    $termOb = new StdClass();
    $termArr = array();
    $defaultType = 0;
    $sql = "SELECT ttd.tid, ttd.name ".
            "FROM {taxonomy_term_data} AS ttd ".
            "WHERE ttd.vid = :vid ORDER BY ttd.weight ASC";
    $result = db_query($sql, array(':vid' => $vid));
    $number_of_rows = $result->rowCount(); //Count the results.
    if($number_of_rows > 0){
        if ($firstTerm != '') {
            $termArr[0] = $firstTerm;
        }
        foreach ($result as $record) {
            $termArr[$record->tid] = $record->name;
        }
    }
    $termOb->list = $termArr;
    $termOb->default = $defaultType;

    return $termOb;
}

/**
* Return the name of a taxonomy tid
* @param $tid (int) : The id of the term
* @return String
*/
function taxonomyName($tid){
    $tidName = '';
    $sql = "SELECT ttd.name ".
           "FROM {taxonomy_term_data} AS ttd ".
           "WHERE ttd.tid = :tid";
    $result = db_query($sql,array(':tid'=>$tid));
    $number_of_rows = $result->rowCount();
    if ($number_of_rows > 0) {
        foreach ($result as $record) {
            $tidName = $record->name;
        }
    }
    return $tidName;
}

/**
* Return taxonomy tid based on the term name
* @param $name (String) - name to search on
* @param $vid (Int) - vocab id to search on
* @return int
*/
function tidLookUp($name,$vid){
    $sql = "SELECT ttd.tid FROM {taxonomy_term_data} AS ttd WHERE ttd.name = :name AND ttd.vid = :vid";
    $result = db_query($sql, array(':name' => $name, ':vid' => $vid));
    $number_of_rows = $result->rowCount(); //Count the results.
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $tid = $record->tid;
        }
    }else{
        $tid = 0;
    }

    return $tid;
}

/**
* Return node nid for a title
* @param $name (String) - title to search on
* @param $type (String) - type of node to search on
* @return int
*/
function nidLookUp($name,$type){
    $name = addslashes($name);
    $sql = "SELECT n.nid FROM {node} AS n WHERE n.type = :type AND n.title = :title";
    $result = db_query($sql,array(':type' => $type, ':title' => $name));
    $number_of_rows = $result->rowCount(); //Count the results.
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $nid = $record->nid;
        }
    }else{
        $nid = 0;
    }

    return $nid;
}

/**
* Return node title for a nid
* @param $nid (int) - nid to search on
* @return string
*/
function titleLookUp($nid){
    $sql = "SELECT n.title FROM {node} AS n WHERE n.nid = :nid";
    $result = db_query($sql,array(':nid' => $nid));
    $number_of_rows = $result->rowCount(); //Count the results.
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $title = $record->title;
        }
    }else{
        $title = '';
    }

    return $title;
}

/**
* Return content type for a nid
* @param $nid (int) - nid to search on
* @return string
*/
function contentType($nid){
    $sql = "SELECT n.type FROM {node} AS n WHERE n.nid = :nid";
    $result = db_query($sql,array(':nid' => $nid));
    $number_of_rows = $result->rowCount(); //Count the results.
    if($number_of_rows > 0){
        foreach ($result as $record) {
            $type = $record->type;
        }
    }else{
        $type = '';
    }

    return $type;
}

/**
* Sort a taxonomy array alphabetically
* @param $arr - (array) - array to sort
* @return array
*/
function _sortArrayAlpha($arr){
    usort($arr, function ($a,$b) {
        return $a['taxonomy_term']->name > $b['taxonomy_term']->name ;
    });
    return $arr;
}

/**
* Return the file extension
*/
function getFileExtension($filename) {
    return preg_match( "/.([^\.]+)$/", $filename, $r ) ? $r[1] : "";
}