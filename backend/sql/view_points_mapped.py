create view
  public.points_mapped as
select
  json_build_object(
    'type',
    'FeatureCollection',
    'features',
    json_agg(st_asgeojson (t.*)::json)
  ) as json_build_object
from
  (
    select
      mapping.closest_point,
      mapping.value
    from
      mapping
  ) t (geom, value);